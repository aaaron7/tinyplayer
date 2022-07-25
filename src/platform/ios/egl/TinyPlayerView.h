#include "core/player_view.hpp"

namespace tinyplayer{
class PlayerViewPlatform : public tinyplayer::PlayerView{
public:
    virtual void Present();
    virtual void PlatformRenderBufferBind();
    virtual void *PlatformView();
    virtual void SetupLayer();
    virtual void Setup();
    virtual ~PlayerViewPlatform();
    void* platform_inst_holder_ = nullptr;
};

}
