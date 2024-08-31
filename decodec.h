#ifndef __DECODEC_H__
#define __DECODEC_H__
#include "packet_queue.h"
#include "type.h"
#include <memory>
#include <thread>
#include <cstdio>
class Decodec {
public:
  Decodec();
  ~Decodec();
  Ret setVolume(int volume);
  void setAudioQueue(std::shared_ptr<PacketQueue> pkt_queue);
  void setVideoQueue(std::shared_ptr<PacketQueue> pkt_queue);
  void Init();
  void StartDecode();

private:
  void DecodeThread();
int adts_header(char * const p_adts_header, const int data_length,
                const int profile, const int samplerate,
                const int channels);
private:
  std::shared_ptr<PacketQueue> audio_pkt_queue_;
  std::shared_ptr<PacketQueue> video_pkt_queue_;
  std::unique_ptr<std::thread> decode_thread_;
  FILE *file_;

};
#endif