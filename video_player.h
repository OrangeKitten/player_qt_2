
#ifndef __VIDEO_PLAYER_H__
#define __VIDEO_PLAYER_H__

#include"demux.h"
#include <memory>
#include "type.h"
#include "decodec.h"
class VideoPlayer {
public:
    VideoPlayer(char *url);
    ~VideoPlayer();
    void Play();
//    void Stop();
//    void Pause();
//    void SetPause(bool pause);
//    bool GetPause();
//    void SetPlay(bool play);
//    bool GetPlay();
//    void SetPosition(int position);
//    int GetPosition();
//    void SetVolume(int volume);
//    int GetVolume();
    Ret  init();
private:
    std::unique_ptr<Demux> demux_;
    std::unique_ptr<Decodec> decodec_;
    std::shared_ptr<PacketQueue>audio_pkt_queue_;
    std::shared_ptr<PacketQueue>video_pkt_queue_;
    bool play_;
    bool pause_;
    int position_;
    int volume_;
    char *url_;
};
#endif
