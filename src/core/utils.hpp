//
//  utils.hpp
//  tinyplayer
//
//  Created by aaron on 2022/7/22.
//

#ifndef utils_hpp
#define utils_hpp
#include <vector>
#include <string>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include "core/frame.hpp"
#include "core/player_view.hpp"
#include <OpenGLES/ES2/gl.h>
#define ENABLE_LOG;
using namespace std;
namespace tinyplayer{
typedef shared_ptr<DecodedFrame> FramePtr;
typedef shared_ptr<DecodedVideoFrame> VideoFramePtr;
typedef shared_ptr<PlayerView> PlayerViewPtr;
typedef vector<shared_ptr<DecodedFrame>> FrameVec;


#define LOG1(format) tinyplayer::log(string("%:Ln %:  ") + format, __FILE__, __LINE__)
#define LOG(format, ...) tinyplayer::log(string("%:Ln %:  ") + format, __FILE__, __LINE__, __VA_ARGS__)
#define MULTILINESTRING(...) #__VA_ARGS__


static inline void log(std::string& format){
#ifdef ENABLE_LOG
    std::cout << format;
#endif
}

template<typename Next, typename ...Other>
void log(std::string& format, const Next& next, const Other&... other){
    auto idx = format.find_first_of("%");
    if (idx == std::string::npos){
        log(format);
    }else{
        std::ostringstream oss;
        oss << next;
        format.replace(idx, 1, oss.str());
        log(format, other...);
    }
}

template <typename ...Args>
void log(const std::string& format, const Args&... args){
    std::string output(format);
    log(output, args...);
    std::cout << endl;
}

template<typename p>
void Apply(vector<p> &arr, p* src, int size){
    for (int i=0;i<size;i++){
        arr[i]= src[i];
    }
}

unique_ptr<uint8_t[]> GetDataFromVideoFrame(unsigned char* data, int lineSize, int width, int height);


class GlHelper{
public:
    static void GenRenderBuffer(GLuint *render_buffer);
    static void CreateFrameBuffer(GLuint render_buffer, GLuint *frame_buffer, GLint *width, GLint *height);
    static void CreateProgram(GLuint *program_handle, GLuint *position_handle);
    static void UploadTexture(GLuint program_handle, FramePtr frame);
    static GLuint LoadShader(GLenum type, string shader_src);
    static void ClearGlContext(GLuint *render_buffer, GLuint *frame_buffer, GLuint *program_handle);
    static vector<GLfloat> GetTexturePos(int video_width, int video_height, int view_width ,int view_height);
    static bool UploadTexture(FramePtr frame, GLuint program_handle, GLuint *textures_, GLint *sampler, GLint video_width, GLint video_height);
};

};
#endif /* utils_hpp */
