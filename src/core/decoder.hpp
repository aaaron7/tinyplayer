//
//  decoder.hpp
//  tinyplayer
//
//  Created by aaron on 2022/7/22.
//

#ifndef decoder_hpp
#define decoder_hpp
#include "core/frame.hpp"
#include "core/utils.hpp"
#include <string>
extern "C" {
    #include "libavformat/avformat.h"
    #include "libavutil/avutil.h"
    #include "libavutil/eval.h"
    #include "libavutil/display.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
}
#include <stdio.h>
using namespace std;
namespace tinyplayer {
class Decoder{
public:
    int Open(string file_url);
    int GetVideoHeight();
    int GetVideoWidth();
    
    bool isEOF();
    void ReadNewFrames(FrameVec &result, FrameType *type);
private:
    AVFormatContext *format_context_;
    AVFrame *vframe_;
    AVFrame *aframe_ = NULL;
    AVCodecContext *video_codec_context_;
    AVCodecContext *audio_codec_context_;
    void *audio_swr_buffer_ = NULL;
    int audio_swr_buffer_size_ = 0;
    int video_index_;
    int audio_index_;
    bool has_video_;
    bool has_audio_;
    bool is_eof_;
    double fps_;
    float audio_sample_rates_ = 48000;
    uint32_t audio_channels_ = 2;
    
    SwrContext *swr_context_ = NULL;
private:
    int FindVideoInfo(AVCodecContext **codec_context);
    int FindAudioInfo(AVCodecContext **codec_context);
    AVCodecContext* FindCodec(int stream_index);
    FrameVec GetFrameFromPacket(AVPacket *packet);
    FrameVec GetAudioFrameFromPacket(AVPacket *packet);
    double GetFPS(AVStream *stream);
    double GetTimeBase(AVStream *stream);
};
}

#endif /* decoder_hpp */
