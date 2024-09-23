#ifndef __PACKET_QUEUE_H__
#define __PACKET_QUEUE_H__

#include <condition_variable>
#include <queue>
#include <mutex>
#include <atomic>
#include <string>
extern "C"
{
#include <libavformat/avformat.h>
}

class PacketQueue {
public:
  PacketQueue(int max_size,std::string queuename);
  ~PacketQueue();
  void Push(void *pkt);
  void* Pop();
//  void PushNullpacket();
  int Size();
  bool Empty();
//  void Clear();
bool Full() ;


private:
  std::mutex mutex_;
  std::condition_variable condition_;
  // std::condition_variable not_full_condition_;
  std::queue< void*> packets_queue_;
  std::atomic<int>  queue_size_;
  int max_size_;
  std::string quque_name_;

};
#endif
