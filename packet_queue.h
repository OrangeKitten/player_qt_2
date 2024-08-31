#ifndef __PACKET_QUEUE_H__
#define __PACKET_QUEUE_H__

#include <condition_variable>
#include <queue>
#include <mutex>

extern "C"
{
#include <libavformat/avformat.h>
}

class PacketQueue {
public:
  PacketQueue();
  ~PacketQueue();
  void Push(AVPacket *pkt);
  AVPacket* Pop();
  void PushNullpacket();
  int Size();
  bool Empty();
  void Clear();
  void wait();
  void notify();


private:
  std::mutex mutex_;
  std::condition_variable condition_;
  std::queue<AVPacket *> packets_queue_;
  int queue_size_;
};
#endif