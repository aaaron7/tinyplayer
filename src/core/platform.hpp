//
//  platform.h
//  tinyplayer
//
//  Created by aaron on 2022/7/22.
//

#ifndef platform_h
#define platform_h
#include "platform/ios/egl/TinyPlayerView.h"
#include "core/audio_render.hpp"

namespace tinyplayer{
AudioRender* CreateAudioRender();
}
#endif /* platform_h */
