//
//  tiny_player.cpp
//  tinyplayer
//
//  Created by aaron on 2022/7/22.
//

#include "tiny_player.hpp"
#include "core/decoder.hpp"
#include<thread>
#include<iostream>
#include <unistd.h>
extern "C" {
    #include "libavformat/avformat.h"
}

using namespace tinyplayer;
Player::Player(){
    Init();
    
}

Player::~Player(){
    
}

void Player::ReleaseThread(){
    if (video_reader_thread_->joinable()){
        video_reader_thread_->join();
    }

    if (video_render_thread_->joinable()){
        video_render_thread_->join();
    }
}


void Player::SetVideoURL(std::string file_url){
    file_url_ = file_url;
}

bool Player::Open(){
    opened_ = false;
    int error = decoder_->Open(file_url_);
    if (error != 0){
        LOG("Open file error for %", error);
    }else
    {
        opened_ = true;
    }
    video_render_->SetVideoSize(decoder_->GetVideoWidth(), decoder_->GetVideoHeight());

    return opened_;
}

void Player::Play(){
    if (!opened_ || playing_){
        assert(0);
         return;
    }

    //TODO: should be removed or not?
    Render();
    playing_ = true;
    if (video_reader_thread_ || video_render_thread_){
        ReleaseThread();
    }

    video_reader_thread_ = unique_ptr<std::thread>(new std::thread(&Player::ReadThreadLoop, this));
    video_render_thread_ = unique_ptr<std::thread>(new std::thread(&Player::RenderThreadLoop, this));
//    renderThread();
}

void Player::Pause(){
    playing_ = false;
}

void Player::Stop(){
    playing_ = false;
}


void Player::Init(){
    decoder_ = std::make_shared<Decoder>();
    
    player_view_ = make_shared<PlayerViewPlatform>();
    player_view_->Setup();

    video_render_ = std::make_shared<VideoRender>(player_view_);

}


void Player::ReadThreadLoop(){
    std::cout<<"start read thread" << std::endl;
    while (playing_){
        ReadFrames();
//        cout<<"into loop"<<endl;
        usleep(100);
    }
}

void Player::RenderThreadLoop() {
    std::cout<<"start render thread" << std::endl;
    while (playing_){
        Render();
        usleep(100);
    }
}

PlayerViewPlatform* Player::GetPlayerView(){
    return player_view_.get();
}

void Player::Render(){

    if (!playing_){
        return ;
    }

    bool eof = decoder_->isEOF();
    bool no_frame = video_frames_.size() <= 0;

    if (no_frame && eof){
        pause();
        LOG1("End of files");
        return;
    }

    if (no_frame){
        usleep(1000);
        return;
    }else{
    }

    if (video_frames_.size() > 0){
        {
            lock_guard<mutex> guard(video_lock_);
            FramePtr frame = video_frames_[0];
            video_frames_.erase(video_frames_.begin());
            LOG1("prepare render frame");

            video_render_->RenderFrame(frame);
        }
//        countFPS();
    }
}

void Player::CountFPS(){
    ++frame_count_;
//    unsigned int curTime = NKM::getCurrentTimeStamp();
//    if (_lastTimeStamp > 0.1){
//        float passed = curTime - _lastTimeStamp;
//        if (passed > 1000){
//            passed /= 1000;
//            float fps = _frameCount / passed;
//            LOG("FPS: %", fps);
//            _lastTimeStamp = curTime;
//            _frameCount = 0;
//        }
//    }else{
//        _lastTimeStamp = curTime;
//    }
}

void Player::ReadFrames(){
//    _buffering = true;
    
    double tmp_duration;
    FrameVec tmp_frame_vec;
    while(playing_ && !decoder_->isEOF()){
        FrameVec fs;
        decoder_->ReadNewFrames(fs);
        if (fs.size() <= 0 ){
            LOG1("Get empty frame from decoder");
            continue;
        }else{
            LOG("Get % frames from decoder", fs.size());
        }

        for_each(fs.begin(), fs.end(), [&](FramePtr& ptr){
            if (ptr->type == FrameTypeVideo){
                tmp_duration += ptr->duration;
                tmp_frame_vec.push_back(ptr);
            }else{
                LOG1("non-video frame found");
            }
        });
        fs.clear();

        {
            lock_guard<mutex> guard(video_lock_);
//            _bufferDuration += tmpDuration;
//            tmpDuration = 0;
            video_frames_.insert(video_frames_.end(), tmp_frame_vec.begin(), tmp_frame_vec.end());
            tmp_frame_vec.clear();
        }
    }

//    _buffering = false;
}
