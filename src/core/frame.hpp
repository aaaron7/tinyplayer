//
//  frame.hpp
//  tinyplayer
//
//  Created by aaron on 2022/7/22.
//

#ifndef frame_hpp
#define frame_hpp
#include <stdint.h>
#include <memory>
#include <stdio.h>
namespace tinyplayer{
using namespace std;
typedef enum {
    FrameTypeUnknow,
    FrameTypeVideo,
    FrameTypeAudio
}FrameType;

class DecodedFrame{
public:
    FrameType type;
    void *buf;
    uint32_t length;
    double duration;
    
    DecodedFrame(){};
    virtual ~DecodedFrame() = default;
};

class DecodedVideoFrame : public DecodedFrame{
public:
    int height;
    int width;
    
    unique_ptr<uint8_t[]> y_data;
    unique_ptr<uint8_t[]> u_data;
    unique_ptr<uint8_t[]> v_data;
    
};


}
#endif /* frame_hpp */
