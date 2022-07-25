//
//  tiny_player.hpp
//  tinyplayer
//
//  Created by aaron on 2022/7/22.
//

#ifndef tiny_player_hpp
#define tiny_player_hpp
#include <string.h>
#include <thread>
#include <stdio.h>
#include "core/player_view.hpp"
#include "core/decoder.hpp"
#include "core/platform.hpp"
#include "core/video_render.hpp"
using namespace std;
namespace tinyplayer{
class Player{
public:
    Player();
    ~Player();
//    NKMPlayerError lastError;

    void SetVideoURL(std::string file_url);

    void Play();
    void Pause();
    void Stop();
    bool Open();
    
    PlayerViewPlatform *GetPlayerView();
    

private:
    std::string file_url_;
    std::shared_ptr<Decoder> decoder_;
    unique_ptr<std::thread> video_reader_thread_;
    unique_ptr<std::thread> video_render_thread_;
    shared_ptr<PlayerViewPlatform> player_view_;
    shared_ptr<VideoRender> video_render_;
    double buffer_duration_;
    FrameVec video_frames_;
    double last_timestamp_ = 0;
    int frame_count_ = 0;

    mutex video_lock_;

    bool opened_ = false;
    bool playing_ = false;
    bool buffering_ = false;
    
    
private:
    void Init();
    void ReadThreadLoop();
    void RenderThreadLoop();
    void ReadFrames();
    void Render();
    void CountFPS();
    void ReleaseThread();
};

}

#endif /* tiny_player_hpp */
