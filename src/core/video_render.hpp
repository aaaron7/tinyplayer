//
//  video_render.hpp
//  tinyplayer
//
//  Created by aaron on 2022/7/22.
//

#ifndef video_render_hpp
#define video_render_hpp
#include "core/player_view.hpp"
#include "core/frame.hpp"
#include "core/utils.hpp"
#include <stdio.h>
namespace tinyplayer{
class VideoRender{
public:
    VideoRender(PlayerViewPtr player_view);
    ~VideoRender();
    
    void SetVideoSize(int width, int height);
    void RenderFrame(FramePtr frame);
    void PrepareRender();
    void CreateBuffer();
    void UpdateCoords();
private:
    PlayerViewPtr player_view_;
    GLuint render_buffer_;
    GLuint frame_buffer_;
    GLuint program_handle_;
    GLuint position_handle_;
    
    GLuint textures_[3];
    GLint sampler_[3];
    
    
    vector<GLfloat> vertex_coords_;
    vector<GLfloat> texture_coords_;
    int video_width_;
    int video_height_;
    
    int render_width_;
    int render_height_;
    
    bool prepared_ = false;

};
}

#endif /* video_render_hpp */
