//
//  decoder.cpp
//  tinyplayer
//
//  Created by aaron on 2022/7/22.
//

#include "decoder.hpp"
#include "core/utils.hpp"
#include <iostream>
using namespace std;

static int ib(void *context){
    std::cout<<"ib triggered" << std::endl;
    return 0;
}
namespace tinyplayer{

int Decoder::Open(string file_url){
    if (file_url.empty()){
        return -1;
    }
    av_register_all();

    avformat_network_init();

    AVFormatContext *format_context = NULL;
    AVDictionary *options = NULL;
    av_dict_set(&options, "probsize", "2147483647", 0);
    av_dict_set(&options, "analyzeduration", "2147483647", 0);

    int ret = avformat_open_input(&format_context, file_url.c_str(), NULL, &options);

    if (ret != 0) {
        if (format_context != NULL){
            avformat_free_context(format_context);
        }

        return -2;
    }

//    av_dump_format(fmtContext, 0 , fileURL.c_str(), 0);
    format_context_ = format_context;

    ret = avformat_find_stream_info(format_context, NULL);
    if (ret < 0){
        avformat_close_input(&format_context);
    }

    AVCodecContext *codec_context = NULL;
    AVFrame *frame = NULL;
    int video_index = FindVideoInfo(&codec_context);
    if (video_index >= 0 && codec_context != NULL){
        AVStream *videoStream = format_context->streams[video_index];
        if (codec_context -> pix_fmt != AV_PIX_FMT_NONE || 1){
            frame = av_frame_alloc();
            bool is_yuv = (codec_context->pix_fmt == AV_PIX_FMT_YUV420P ||
                            codec_context->pix_fmt == AV_PIX_FMT_YUVJ420P);
            if (!is_yuv){
                // non-yuv frame is currently not supported
                assert(0);
            }

            fps_ = GetFPS(videoStream);
//            _timebase = getTimeBase(videoStream);
        }

        if (frame == NULL){
            video_index = -1;
            if (codec_context != NULL)
                avcodec_free_context(&codec_context);
        }
    }


    //ToDO: Find audio

    if (video_index < 0) {
        avformat_close_input(&format_context);
        return -3;
    }

    avframe_ = frame;
    video_index_ = video_index;
    codec_context_ = codec_context;

    AVIOInterruptCB icb = {ib, NULL};
    format_context->interrupt_callback = icb;
    
//    FrameVec fs;
//    this->ReadNewFrames(fs);

    return 0;
}

int Decoder::FindVideoInfo(AVCodecContext **codec_context){
    int index = -1;
    for (int i = 0; i < format_context_->nb_streams; i++){
        if (format_context_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            AVCodecContext *codecctx = FindVideoCodec(i);
            if (codecctx != NULL){
                if (codec_context != NULL){
                    *codec_context = codecctx;
                    index = i;
                    break;
                }
            }
        }
    }
    return index;
}



AVCodecContext* Decoder::FindVideoCodec(int stream_index){
    AVCodecParameters *params = format_context_->streams[stream_index]->codecpar;
    AVCodec *codec = avcodec_find_decoder(params->codec_id);
    if (codec == NULL){
        return NULL;
    }
    
    AVCodecContext *context = avcodec_alloc_context3(codec);
    if (context == NULL){
        return NULL;
    }
    
    int ret = avcodec_parameters_to_context(context, params);
    if (ret < 0){
        avcodec_free_context(&context);
        return NULL;
    }
    
    ret = avcodec_open2(context, codec, NULL);
    if (ret < 0){
        avcodec_free_context(&context);
        return NULL;
    }
    
    return context;

}

void Decoder::ReadNewFrames(FrameVec &result){
    AVPacket packet;
    av_init_packet(&packet);

    bool isReading = true;
    while(isReading){
        int ret = av_read_frame(format_context_, &packet);
        if (ret < 0){
            if (ret == AVERROR_EOF){
                is_eof_ = true;
                char *errorInfo = av_err2str(ret);
                LOG1(errorInfo);
            }
            break;
        }

        FrameVec frames;
        if (packet.stream_index == video_index_){
            frames = GetFrameFromPacket(&packet);
            isReading = false;
        }

        if (frames.size() > 0){
            result.insert(result.end(), frames.begin(), frames.end());
            
            frames.clear();
        }
        av_packet_unref(&packet);
    }
    
}

FrameVec Decoder::GetFrameFromPacket(AVPacket *packet){
    FrameVec vec;
    if (!codec_context_){
        assert(0);
        return vec;
    }
    
    int ret = avcodec_send_packet(codec_context_, packet);
    if (ret != 0){
        LOG("avcodec send packet: %", ret);
        return vec;
    }
    
    do{
        ret = avcodec_receive_frame(codec_context_, avframe_);
        if (ret == AVERROR_EOF){
            LOG1("AVEOF");
            break;
        } else if (ret == AVERROR(EAGAIN)){
            LOG("averror eagain %", ret);
            break;
        } else if (ret <0){
            LOG("avcodec receive frame: %", ret);
            break;
        }
        
        int width = codec_context_ ->width;
        int height = codec_context_ ->height;
        
        FramePtr frame(new DecodedVideoFrame());
        VideoFramePtr vf = dynamic_pointer_cast<DecodedVideoFrame>(frame);
        vf->width = codec_context_->width;
        vf->height = codec_context_->height;
        vf->type = FrameTypeVideo;
        vf->duration = avframe_->pkt_duration>0?:1/fps_;
        
        vf->y_data = GetDataFromVideoFrame(avframe_->data[0], avframe_->linesize[0], width, height);
        vf->u_data = GetDataFromVideoFrame(avframe_->data[1], avframe_->linesize[1], width / 2, height / 2);
        vf->v_data = GetDataFromVideoFrame(avframe_->data[2], avframe_->linesize[2], width / 2, height / 2);
        
        
        vec.push_back(frame);
        
    }while(true);
    
    return vec;
}

int Decoder::GetVideoWidth(){
    if (codec_context_ == NULL){
        return 0;
    }
    int width = codec_context_->width;
    AVRational sar = codec_context_->sample_aspect_ratio;
    if (sar.num != 0 && sar.den != 0){
        width = width * sar.num / sar.den;
    }

    return width;
}

int Decoder::GetVideoHeight(){
    if (codec_context_ == NULL){
        return 0;
    }


    int height = codec_context_->height;
    return height;
}

double Decoder::GetFPS(AVStream *stream){
    double f = 0;
    if (stream->avg_frame_rate.den > 0 && stream->avg_frame_rate.num > 0){
        f = av_q2d(stream->avg_frame_rate);
    }else if (stream->r_frame_rate.den > 0 && stream->r_frame_rate.num > 0){
        f = av_q2d(stream->r_frame_rate);
    }else{
        f = 1 / GetTimeBase(stream);
    }

    return f;
}

double Decoder::GetTimeBase(AVStream *stream){
    double t = 0;
    if (stream ->time_base.den > 0 && stream->time_base.num > 0){
        t = av_q2d(stream->time_base);
    }else {
        // default time base
        t = 0.04;
    }
    return t;
}

bool Decoder::isEOF(){
    return is_eof_;
}
}
