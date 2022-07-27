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
    int audio_index = FindAudioInfo(&audio_codec_context_);
    if (audio_index >=0 && audio_codec_context_ != NULL){
        swr_context_ = swr_alloc_set_opts(NULL, av_get_default_channel_layout(audio_channels_), AV_SAMPLE_FMT_S16, audio_sample_rates_, av_get_default_channel_layout(audio_codec_context_->channels), audio_codec_context_->sample_fmt, audio_codec_context_->sample_rate, 0, NULL);
        int ret = swr_init(swr_context_);
        if (ret < 0){
            assert(0);
        }
    }
    

    if (video_index < 0) {
        avformat_close_input(&format_context);
        return -3;
    }

    vframe_ = frame;
    aframe_ = av_frame_alloc();
    video_index_ = video_index;
    audio_index = audio_index;
    video_codec_context_ = codec_context;

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
            AVCodecContext *codecctx = FindCodec(i);
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

int Decoder::FindAudioInfo(AVCodecContext **codec_context){
    int index = -1;
    for (int i = 0; i < format_context_->nb_streams; i++){
        if (format_context_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
            AVCodecContext *codecctx = FindCodec(i);
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


AVCodecContext* Decoder::FindCodec(int stream_index){
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


void Decoder::ReadNewFrames(FrameVec &result,FrameType *type){
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
            *type = FrameTypeVideo;
        }else{
            LOG1("get audio packets");
            frames = GetAudioFrameFromPacket(&packet);
            isReading = false;
            *type = FrameTypeAudio;
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
    if (!video_codec_context_){
        assert(0);
        return vec;
    }
    
    int ret = avcodec_send_packet(video_codec_context_, packet);
    if (ret != 0){
        LOG("avcodec send packet: %", ret);
        return vec;
    }
    
    do{
        ret = avcodec_receive_frame(video_codec_context_, vframe_);
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
        
        int width = video_codec_context_ ->width;
        int height = video_codec_context_ ->height;
        
        FramePtr frame(new DecodedVideoFrame());
        VideoFramePtr vf = dynamic_pointer_cast<DecodedVideoFrame>(frame);
        vf->width = video_codec_context_->width;
        vf->height = video_codec_context_->height;
        vf->type = FrameTypeVideo;
        vf->duration = vframe_->pkt_duration>0?:1/fps_;
        
        vf->y_data = GetDataFromVideoFrame(vframe_->data[0], vframe_->linesize[0], width, height);
        vf->u_data = GetDataFromVideoFrame(vframe_->data[1], vframe_->linesize[1], width / 2, height / 2);
        vf->v_data = GetDataFromVideoFrame(vframe_->data[2], vframe_->linesize[2], width / 2, height / 2);
        
        
        vec.push_back(frame);
        
    }while(true);
    
    return vec;
}


FrameVec Decoder::GetAudioFrameFromPacket(AVPacket *packet){
    FrameVec vec;
    if (!audio_codec_context_){
        assert(0);
    }
    int ret = avcodec_send_packet(audio_codec_context_, packet);
    if (ret != 0){
        LOG("decoder error: %", ret);
    }
    
    do {
        ret =avcodec_receive_frame(audio_codec_context_, aframe_);
        if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)){
            break;
        }else if (ret < 0){
            LOG("receive frame: %", ret);
            break;
        }
        
        if (aframe_->data[0] == NULL){
            continue;;
        }
        
        uint8_t *data = NULL;
        int sample_per_channel =0;
        if (swr_context_ != NULL){
            float sample_ratio = audio_sample_rates_ / audio_codec_context_->sample_rate;
            float channel_ratio = audio_channels_ / audio_codec_context_->channels;
            float ratio = max(1.0f, sample_ratio) * max(1.0f, channel_ratio) ;
            
            int samples = aframe_->nb_samples * ratio;
            int bufsize = av_samples_get_buffer_size(NULL,
                                                     audio_channels_,
                                                     samples,
                                                     AV_SAMPLE_FMT_S16,
                                                     1);
            if (audio_swr_buffer_ == NULL || audio_swr_buffer_size_ < bufsize) {
                audio_swr_buffer_size_ = bufsize;
                audio_swr_buffer_ = realloc(audio_swr_buffer_, bufsize);
            }
            
            uint8_t *o[2] = { (uint8_t*)audio_swr_buffer_, 0 };
            sample_per_channel = swr_convert(swr_context_, o, samples, (const uint8_t **)aframe_->data, aframe_->nb_samples);
            if (sample_per_channel < 0) {
                LOG1("resample failed");
                return vec;
            }
            
            data = (uint8_t *)audio_swr_buffer_;
        }else{
            if (audio_codec_context_->sample_fmt != AV_SAMPLE_FMT_S16){
                assert(0);
                return vec;
            }
            
            data = aframe_->data[0];
            sample_per_channel = aframe_->nb_samples;
        }
        
        FramePtr frame(new DecodedFrame());
        int data_length = sample_per_channel * audio_channels_ * sizeof(float);
        int elements =sample_per_channel * audio_channels_;
        frame->buf = new uint8_t[audio_swr_buffer_size_];
        memcpy(frame->buf, data, audio_swr_buffer_size_);
        frame->length = audio_swr_buffer_size_;
//        for (int i = 0; i < elements; i++) {
//            float jj = (float)data[i] / INT16_MAX;
//
//            ((float *)frame->buf)[i] = jj;
//        }
//        float scalar = 1.0f / INT16_MAX;
//
//        vDSP_vflt16((short *)data, 1, (float *)frame->buf, 1, elements);
//        vDSP_vsmul((float *)frame->buf, 1, &scalar, (float *)frame->buf, 1, elements);
        

        vec.push_back(frame);
//        frame->duration = aframe_->pkt_duration *
        
    } while (true);
    return vec;
    
}

int Decoder::GetVideoWidth(){
    if (video_codec_context_ == NULL){
        return 0;
    }
    int width = video_codec_context_->width;
    AVRational sar = video_codec_context_->sample_aspect_ratio;
    if (sar.num != 0 && sar.den != 0){
        width = width * sar.num / sar.den;
    }

    return width;
}

int Decoder::GetVideoHeight(){
    if (video_codec_context_ == NULL){
        return 0;
    }


    int height = video_codec_context_->height;
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
