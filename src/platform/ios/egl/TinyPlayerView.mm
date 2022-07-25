#import "TinyPlayerView.h"
#import <UIKit/UIKit.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

@interface TinyPlayerView : UIView

@end





@interface TinyPlayerView () {
    CAEAGLLayer *_glLayer;
    EAGLContext *_glContext;
    tinyplayer::PlayerViewPlatform *_playerView;
}
+ (instancetype)sharedInstance;
- (void)present;
- (void)bind;
- (void)setupLayers;
- (void)setPlayerView:(tinyplayer::PlayerViewPlatform *)playerView;
@end
using namespace tinyplayer;
@implementation TinyPlayerView

- (instancetype)init{
    self = [super init];
    if (self){
//        [self setupLayers];
    }
    
    return self;
}

+(Class)layerClass{
    return [CAEAGLLayer class];
}

- (void)setupLayers{
    dispatch_sync(dispatch_get_main_queue(), ^{
        _glLayer = (CAEAGLLayer *)self.layer;
    });
    _glLayer.opaque = YES;
    _glLayer.drawableProperties = @{kEAGLDrawablePropertyRetainedBacking : @(NO), kEAGLDrawablePropertyColorFormat : kEAGLColorFormatRGBA8};
    _glContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    if (_glContext == nil){
        assert(0);
        return;
    }

    if (![EAGLContext setCurrentContext:_glContext]){
        return;
    }
}

- (void)initLayers{
    
}

- (void)bind{
    [CATransaction flush];
//    NSLog(@"frame:%@", NSStringFromCGRect(self.frame));
    BOOL result = [_glContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:_glLayer];
    assert(result);
}

- (void)layoutSubviews
{
//    _playerView->reset();
}

- (void)present{
    [_glContext presentRenderbuffer:GL_RENDERBUFFER];
}

- (void)setPlayerView:(PlayerViewPlatform *)playerView
{
    _playerView = playerView;
}

@end


PlayerViewPlatform::~PlayerViewPlatform(){
    TinyPlayerView *view = (__bridge TinyPlayerView *)this->platform_inst_holder_;
    [view release];
}

void PlayerViewPlatform::Present(){
    TinyPlayerView *view = (__bridge TinyPlayerView *)this->platform_inst_holder_;
    [view present];
}

void PlayerViewPlatform::PlatformRenderBufferBind(){
    TinyPlayerView *view = (__bridge TinyPlayerView *)this->platform_inst_holder_;
    [view bind];
}

void* PlayerViewPlatform::PlatformView(){
    return this->platform_inst_holder_;
}

void PlayerViewPlatform::Setup(){
    TinyPlayerView *playerView =[[TinyPlayerView alloc] init];
    this->platform_inst_holder_ = (__bridge void *)playerView;
    [playerView setPlayerView:this];
    [playerView retain];
}


void PlayerViewPlatform::SetupLayer(){
    TinyPlayerView *view = (__bridge TinyPlayerView *)this->platform_inst_holder_;
    [view setupLayers];
}
