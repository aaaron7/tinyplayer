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
    void ReadNewFrames(FrameVec &result);
private:
    AVFormatContext *format_context_;
    AVFrame *avframe_;
    AVCodecContext *codec_context_;
    int video_index_;
    int audio_index_;
    bool has_video_;
    bool has_audio_;
    bool is_eof_;
    double fps_;
    int FindVideoInfo(AVCodecContext **codec_context);
    AVCodecContext* FindVideoCodec(int stream_index);
    FrameVec GetFrameFromPacket(AVPacket *packet);
    
    double GetFPS(AVStream *stream);
    double GetTimeBase(AVStream *stream);
};
}

#endif /* decoder_hpp */
