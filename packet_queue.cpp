#include "packet_queue.h"

extern "C"
{
#include "log.h"
#include <libavformat/avformat.h>
}


PacketQueue::PacketQueue() { int queue_size_ = -1; }
PacketQueue::~PacketQueue() { log_debug("PacketQueue Release!"); }

void PacketQueue::Push(AVPacket *pkt) {
  std::lock_guard<std::mutex> lock(mutex_);
  packets_queue_.push(pkt);
  queue_size_++;
  condition_.notify_all();
}

AVPacket* PacketQueue::Pop() {
  std::unique_lock<std::mutex> lock(mutex_);
  while (packets_queue_.empty()) {
            condition_.wait(lock);
        }
  AVPacket* pkt = nullptr;
  if (!packets_queue_.empty()) {
    pkt = packets_queue_.front();
    packets_queue_.pop();
    queue_size_--;
  }
  return pkt;
}
void PacketQueue::PushNullpacket() {
  std::lock_guard<std::mutex> lock(mutex_);
  AVPacket *pkt = av_packet_alloc();
  av_init_packet(pkt);
  packets_queue_.push(pkt);
  queue_size_++;
  condition_.notify_all();
}
int PacketQueue::Size() {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_size_;
}
bool PacketQueue::Empty() {
  std::lock_guard<std::mutex> lock(mutex_);
  return packets_queue_.empty();
}
void PacketQueue::Clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  while (!packets_queue_.empty()) {
    AVPacket *pkt = packets_queue_.front();
    packets_queue_.pop();
    av_packet_unref(pkt);
    av_packet_free(&pkt);
    queue_size_--;
  }
}
void PacketQueue::wait() {
  std::unique_lock<std::mutex> lock(mutex_);
  condition_.wait(lock, [this]() { return !packets_queue_.empty(); });
}
void PacketQueue::notify() {
  std::lock_guard<std::mutex> lock(mutex_);
  condition_.notify_all();
}
