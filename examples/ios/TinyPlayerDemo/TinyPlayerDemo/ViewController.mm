//
//  ViewController.m
//  TinyPlayerDemo
//
//  Created by aaron on 2022/7/25.
//

#import "ViewController.h"
#import "tiny_player.hpp"
#import "player_view.hpp"
#import "platform.hpp"
@interface ViewController (){
    tinyplayer::Player *tinyPlayer_;
}
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    tinyPlayer_ = new tinyplayer::Player();

    // Do any additional setup after loading the view.
}

- (void)viewDidLayoutSubviews{
    UIView *view = (__bridge UIView *)tinyPlayer_->GetPlayerView()->PlatformView();
    view.frame = self.view.bounds;
    [self.view addSubview:view];
    self.view.backgroundColor = [UIColor blueColor];
    NSString *path = [[NSBundle mainBundle] pathForResource:@"02_Skater" ofType:@"mp4"];
//    NSString *path = [[NSBundle mainBundle] pathForResource:@"04_Aerial" ofType:@"mp4"];
    tinyPlayer_->SetVideoURL([path UTF8String]);
    if (tinyPlayer_->Open()){
        tinyPlayer_->Play();
    }else{
        assert(0);
    }
}


@end
