//
//  player_view.h
//  tinyplayer
//
//  Created by aaron on 2022/7/22.
//

#ifndef player_view_h
#define player_view_h

namespace tinyplayer {
class PlayerView{
    
public:
    PlayerView():is_inited_(false){
        
    }
    
    void PrepareLayers(){
        if (!is_inited_){
            is_inited_ = true;
            SetupLayer();
        }
    }
    virtual void PlatformRenderBufferBind(){};
    virtual void Present(){};
    virtual void Setup(){};
    
protected:

    
    virtual void SetupLayer(){};
    
    virtual ~PlayerView() = default;
    
private:
    bool is_inited_;
    
};

}

#endif /* player_view_h */
