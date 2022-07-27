//
//  audio_unit_render.cpp
//  tinyplayer
//
//  Created by aaron on 2022/7/26.
//

#include "audio_unit_render.hpp"
#include "core/tiny_player.hpp"
#include "core/platform.hpp"
#import <Accelerate/Accelerate.h>

namespace tinyplayer {

AudioRender *CreateAudioRender(){
    return new AudioUnitRender();
}

bool AudioUnitRender::Init(){
    
    
    buffer_ = (float *)calloc(4096 * 2, sizeof(float));
    
    AVAudioSession *session = [AVAudioSession sharedInstance];
    NSError *rawError = nil;
    if (![session setCategory:AVAudioSessionCategoryPlayback error:&rawError]) {

        return NO;
    }
    
    NSTimeInterval prefferedIOBufferDuration = 0.023;
    if (![session setPreferredIOBufferDuration:prefferedIOBufferDuration error:&rawError]) {
        NSLog(@"setPreferredIOBufferDuration: %.4f, error: %@", prefferedIOBufferDuration, rawError);
    }
    
    double prefferedSampleRate = 44100;
    if (![session setPreferredSampleRate:prefferedSampleRate error:&rawError]) {
        NSLog(@"setPreferredSampleRate: %.4f, error: %@", prefferedSampleRate, rawError);
    }
    
    if (![session setActive:YES error:&rawError]) {
        return NO;
    }
    
    float volume = session.outputVolume;
    if (volume < 0) {
        return NO;
    }
    
    double sampleRate = session.sampleRate;
    NSInteger channels = session.outputNumberOfChannels;

    
    AudioComponentDescription descr = {0};
    descr.componentType = kAudioUnitType_Output;
    descr.componentSubType = kAudioUnitSubType_RemoteIO;
    descr.componentManufacturer = kAudioUnitManufacturer_Apple;
    descr.componentFlags = 0;
    descr.componentFlagsMask = 0;
    
    AudioUnit audiounit = NULL;
    AudioComponent component = AudioComponentFindNext(NULL, &descr);
    OSStatus status = AudioComponentInstanceNew(component, &audiounit);
    if (status != noErr) {
        NSError *rawError = [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
       
        return NO;
    }
    
    AudioStreamBasicDescription streamDescr = {0};
    UInt32 size = sizeof(AudioStreamBasicDescription);
    status = AudioUnitGetProperty(audiounit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input,
                                  0, &streamDescr, &size);
    if (status != noErr) {
        NSError *rawError = [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];

        return NO;
    }
    
    streamDescr.mSampleRate = sampleRate;
    status = AudioUnitSetProperty(audiounit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input,
                                  0, &streamDescr, size);
    if (status != noErr) {
        NSLog(@"FAILED to set audio sample rate: %f, error: %d", sampleRate, (int)status);
    }
    
    bits_per_channel_ = streamDescr.mBitsPerChannel;
    channels_per_frame_ = streamDescr.mChannelsPerFrame;
    
    AURenderCallbackStruct renderCallbackStruct;
    renderCallbackStruct.inputProc = renderCallback;
    renderCallbackStruct.inputProcRefCon = this;
    
    status = AudioUnitSetProperty(audiounit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &renderCallbackStruct, sizeof(AURenderCallbackStruct));
    if (status != noErr) {
        NSError *rawError = [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];

        return NO;
    }
    
    status = AudioUnitInitialize(audiounit);
    if (status != noErr) {
        NSError *rawError = [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];

        return NO;
    }
    
    audio_unit_ = audiounit;
    inited_ = true;
}

bool AudioUnitRender::Play(){
    if (!inited_){
        assert(0);
    }
    
    OSStatus status = AudioOutputUnitStart(audio_unit_);
    if (status == noErr){
        playing_ = true;
    }
    return playing_;
}

OSStatus AudioUnitRender::renderCallback(void* inRefCon, AudioUnitRenderActionFlags* inActionFlags,
                                                        const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber,
                                                        UInt32 inNumberFrames, AudioBufferList* ioData)
{

    UInt32 num = ioData->mNumberBuffers;
    for (UInt32 i = 0; i < num; ++i) {
        AudioBuffer buf = ioData->mBuffers[i];
        memset(buf.mData, 0, buf.mDataByteSize);
    }
    AudioUnitRender *render = (AudioUnitRender *)inRefCon;
    if (!render ->player_ref){
        assert(0);
    }
    render->player_ref->RenderAudioFrame(render->buffer_,inNumberFrames, render->channels_per_frame_);
    for (int i = 0 ; i< ioData->mNumberBuffers ; i++){
        AudioBuffer buf = ioData->mBuffers[i];
        uint32_t channels = buf.mNumberChannels;
        float scalar = 0;

        for (int j = 0;j<channels; ++j){


            vDSP_vsadd((float *)render->buffer_ + i + j, render->channels_per_frame_, &scalar, (float *)buf.mData + j, channels, inNumberFrames);
//            for (int k = 0; k < inNumberFrames;k++){
//                LOG("data: %",(float)*((float*)buf.mData + j + k));
//            }
        }
    }
    return noErr;
}
}
