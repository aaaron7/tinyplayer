//
//  audio_render.hpp
//  tinyplayer
//
//  Created by aaron on 2022/7/26.
//

#ifndef audio_render_hpp
#define audio_render_hpp

#include <stdio.h>
#include <memory>
namespace tinyplayer{
using namespace std;
class Player;
class AudioRender{
public:
    AudioRender(){
        
    }
    Player* player_ref;
    virtual ~AudioRender() = default;
    virtual bool Init(){return true;}
    virtual bool Play(){return true;}
    virtual bool Stop(){return true;}
    
};
}
#endif /* audio_render_hpp */
