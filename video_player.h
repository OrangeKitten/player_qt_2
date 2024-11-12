
#ifndef __VIDEO_PLAYER_H__
#define __VIDEO_PLAYER_H__

#include"demux.h"
#include <memory>
#include "type.h"
#include "decodec.h"
#include <mutex>
class VideoPlayer {
public:
    VideoPlayer(char *url);
    ~VideoPlayer();
    void Play();
    void Stop();
    void Pause();
//    void SetPause(bool pause);
//    bool GetPause();
//    void SetPlay(bool play);
//    bool GetPlay();
//    void SetPosition(int position);
//    int GetPosition();
   void set_volume(int volume);
   int get_volume();
   Ret set_mute(bool mute);
    Ret  init();
    PlayerState getState();
private:
    std::unique_ptr<Demux> demux_;
    std::unique_ptr<Decodec> decodec_;
    std::shared_ptr<PacketQueue>audio_pkt_queue_;
    std::shared_ptr<PacketQueue>video_pkt_queue_;
    PlayerState player_state_;
    bool pause_state_;
    bool last_pause_state_;
    int position_;
    int volume_;
    char *url_;
    std::mutex mtx_;
};
#endif
