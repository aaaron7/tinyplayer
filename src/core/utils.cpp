//
//  utils.cpp
//  tinyplayer
//
//  Created by aaron on 2022/7/22.
//

#include "utils.hpp"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

namespace tinyplayer {

static string VertexShader = MULTILINESTRING(
    attribute vec4 position;
    attribute vec2 texcoord;
    uniform mat4 projection;
    varying vec2 v_texcoord;

    void main() {
        gl_Position = position;
        v_texcoord = texcoord;
    }
);

static string FragementShader = MULTILINESTRING(
varying highp vec2 v_texcoord;
    uniform sampler2D s_texture_y;
    uniform sampler2D s_texture_u;
    uniform sampler2D s_texture_v;

    void main() {
        highp float y = texture2D(s_texture_y, v_texcoord).r;
        highp float u = texture2D(s_texture_u, v_texcoord).r - 0.5;
        highp float v = texture2D(s_texture_v, v_texcoord).r - 0.5;
        
        highp float r = y + 1.402 * v;
        highp float g = y - 0.344 * u - 0.714 * v;
        highp float b = y + 1.772 * u;
        
        gl_FragColor = vec4(r, g, b, 1);
    }
);
    

unique_ptr<uint8_t[]> GetDataFromVideoFrame(unsigned char* data, int lineSize, int width, int height){
    width = min(lineSize, width);

    int size = width * height;
    unique_ptr<uint8_t[]> buffer(new uint8_t[size]);
    uint8_t *bytePointer = buffer.get();
    for (int i = 0 ; i < height ; ++ i){
        memcpy(bytePointer, data, width);
        bytePointer += width;
        data += lineSize;
    }

    return buffer;
}

void GlHelper::CreateFrameBuffer(GLuint render_buffer, GLuint *frame_buffer, GLint *width, GLint *height){
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, height);
    
    
    glGenFramebuffers(1, frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, *frame_buffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, render_buffer);
    
}

void GlHelper::GenRenderBuffer(GLuint *render_buffer){
    glGenRenderbuffers(1, render_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, *render_buffer);
}


GLuint GlHelper::LoadShader(GLenum type, string shader_src){
    GLuint shader = glCreateShader(type);
    if (shader == 0){
        LOG1("SHADER CREATE FAILED");
        return 0;
    }

    const char *shaderChars = shader_src.c_str();
    glShaderSource(shader, 1, &shaderChars, NULL);

    glCompileShader(shader);

    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (compiled == 0 ){
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        if (length > 1){
            char *log = (char*)malloc(sizeof(char) * length);
            glGetShaderInfoLog(shader, length, NULL, log);
            LOG("%", log);
            free(log);
        }
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

void GlHelper::CreateProgram(GLuint *program_handle, GLuint *position_handle){
    GLuint program = glCreateProgram();
    if (program == 0){
        LOG1("GL program createation failed");
        assert(0);
        return ;
    }

    //TODO: load rotate and scale shader
    GLuint vertexShader = LoadShader(GL_VERTEX_SHADER, VertexShader);
    GLuint fragShader = LoadShader(GL_FRAGMENT_SHADER, FragementShader);

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragShader);

    glBindAttribLocation(program, 0, "position");
    glBindAttribLocation(program, 1, "texcoord");

    glLinkProgram(program);

    GLint linked = 0 ;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked == 0){
        GLint length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        if (length > 1){
            char *log = (char *)malloc(sizeof(char) * length);
            glGetProgramInfoLog(program,length, NULL, log);
            LOG("%", log);
            free(log);
        }
        glDeleteProgram(program);
        return;
    }

    glUseProgram(program);
    *position_handle = glGetAttribLocation(program, "position");

    *program_handle = program;
}

void GlHelper::ClearGlContext(GLuint *render_buffer, GLuint *frame_buffer, GLuint *program_handle){
    glDeleteBuffers(1, render_buffer);
    *render_buffer = 0;
    
    glDeleteBuffers(1, frame_buffer);
    *frame_buffer = 0;
    
    glDeleteProgram(*program_handle);
    *program_handle = 0;
    
    
}


vector<GLfloat> GlHelper::GetTexturePos(int video_width, int video_height, int view_width ,int view_height){
    vector<GLfloat> result;
    if (video_width == 0 || video_height == 0){
        float vertex[] = {-1, -1, 1,-1,-1,1,1,1};
        Apply(result, vertex, 8);
        return result;
    }
    
    float ratio_w = (float)view_width / video_width;
    float ratio_h = (float)view_height / video_height;
    float ratio = min(ratio_w, ratio_h);

    const float w = video_width * ratio / view_width;
    const float h = video_height * ratio / view_height;
    float vertex[] = {-w,-h,w,-h,-w,h, w,h};
    result.resize(8);
    Apply(result, vertex, 8);
    return result;

}


bool GlHelper::UploadTexture(FramePtr frame, GLuint program_handle, GLuint *textures, GLint *sampler, GLint video_width, GLint video_height){
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    glGenTextures(3, textures);
    if (textures[0] == 0){
        assert(0);
        return false;
    }
    VideoFramePtr video_frame = dynamic_pointer_cast<DecodedVideoFrame>(frame);
    const uint8_t *data[3] = {video_frame->y_data.get(), video_frame->u_data.get(), video_frame->v_data.get()};
    const int width_arr[3] = {video_width, video_width / 2, video_width / 2};
    const int height_arr[3] = {video_height, video_height / 2, video_height / 2};
    for (int i = 0 ; i < 3 ; ++i ){
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_LUMINANCE,
                     width_arr[i],
                     height_arr[i],
                     0,
                     GL_LUMINANCE,
                     GL_UNSIGNED_BYTE,
                     data[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
    }

    if (sampler[0] == -1){
        sampler[0] = glGetUniformLocation(program_handle, "s_texture_y");
        if (sampler[0] == -1)
            return false;
    }

    if (sampler[1] == -1){
        sampler[1] = glGetUniformLocation(program_handle, "s_texture_u");
        if (sampler[1] == -1)
            return false;
    }

    if (sampler[2] == -1){
        sampler[2] = glGetUniformLocation(program_handle, "s_texture_v");
        if (sampler[2] == -1)
            return false;
        
    }
    
    for (int i = 0 ; i < 3 ; ++i){
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glUniform1i(sampler[i], i);
    }

    return true;
}
}
#pragma clang diagnostic pop
