//
//  video_render.cpp
//  tinyplayer
//
//  Created by aaron on 2022/7/22.
//

#include "video_render.hpp"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

namespace tinyplayer {
VideoRender::VideoRender(PlayerViewPtr player_view):player_view_(player_view),
                                                    video_width_(0), video_height_(0),
                                                    render_width_(0),
                                                    render_height_(0),
                                                    texture_coords_{0,1,1,1,0,0,1,0},
                                                    textures_{0,0,0},
                                                    sampler_{-1,-1,-1}
{
    
}

VideoRender::~VideoRender(){
    
}


void VideoRender::SetVideoSize(int width, int height){
    video_width_ = width;
    video_height_ = height;

}

void VideoRender::PrepareRender(){
    player_view_->PrepareLayers();
    CreateBuffer();
    GlHelper::CreateProgram(&program_handle_, &position_handle_);
//    CreateBuffer();
    UpdateCoords();
    
}

void VideoRender::CreateBuffer(){
    GlHelper::GenRenderBuffer(&render_buffer_);
    player_view_->PlatformRenderBufferBind();
    GlHelper::CreateFrameBuffer(render_buffer_, &frame_buffer_, &render_width_, &render_height_);
}

void VideoRender::RenderFrame(FramePtr frame){
    if (!prepared_){
        prepared_ = true;
        PrepareRender();
    }
    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glViewport(0,0,render_width_, render_height_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    bool succ = GlHelper::UploadTexture(frame, program_handle_, textures_, sampler_, video_width_, video_height_);
    if (succ){
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, &vertex_coords_[0]);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, &texture_coords_[0]);
        glEnableVertexAttribArray(1);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }else{
        assert(0);
    }
    
    player_view_->Present();
    
    GLenum error = glGetError();
    LOG("gl error: %", error);
}

void VideoRender::UpdateCoords(){
    
    vertex_coords_.clear();
    vertex_coords_ = GlHelper::GetTexturePos(video_width_, video_height_, render_width_, render_height_);
    
}
}
#pragma clang diagnostic pop
