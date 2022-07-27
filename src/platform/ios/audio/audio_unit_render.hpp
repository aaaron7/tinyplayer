//
//  audio_unit_render.hpp
//  tinyplayer
//
//  Created by aaron on 2022/7/26.
//

#ifndef audio_unit_render_hpp
#define audio_unit_render_hpp
#import "core/audio_render.hpp"
#include <stdio.h>
#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioToolbox.h>
using namespace std;
class Player;
namespace tinyplayer{
class AudioUnitRender : public AudioRender{
public:
    virtual bool Init();
    virtual bool Play();
    virtual bool Stop();
    static OSStatus renderCallback(void* inRefCon, AudioUnitRenderActionFlags* inActionFlags,
                                                            const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber,
                                   UInt32 inNumberFrames, AudioBufferList* ioData);
    uint32_t channels_per_frame_;
    short *buffer_;


private:
    AudioUnit audio_unit_;
    uint32_t bits_per_channel_;
    bool inited_ = false;
    bool playing_ = false;
};
}
#endif /* audio_unit_render_hpp */
