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
#include "core/platform.hpp"
#include "core/video_render.hpp"
#include "core/audio_render.hpp"
using namespace std;
namespace tinyplayer{
class Decoder;
class Player{
public:
    Player();
    ~Player();

    void SetVideoURL(std::string file_url);

    void Play();
    void Pause();
    void Stop();
    bool Open();
    
    PlayerViewPlatform *GetPlayerView();
    
    void RenderAudioFrame(float *data, uint32_t frames, uint32_t channels);

private:
    std::string file_url_;
    std::shared_ptr<Decoder> decoder_;
    unique_ptr<std::thread> video_reader_thread_;
    unique_ptr<std::thread> video_render_thread_;
    shared_ptr<PlayerViewPlatform> player_view_;
    shared_ptr<VideoRender> video_render_;
    shared_ptr<AudioRender> audio_render_;
    double buffer_duration_;
    FrameVec video_frames_;
    FrameVec audio_frames_;
    double last_timestamp_ = 0;
    int frame_count_ = 0;

    mutex video_lock_;
    mutex audio_lock_;

    bool opened_ = false;
    bool playing_ = false;
    bool buffering_ = false;
    
    FramePtr current_audio_frame_ = nullptr;
    int current_audio_frame_offset_;
    
    
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
