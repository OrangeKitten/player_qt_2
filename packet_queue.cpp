#include "packet_queue.h"

extern "C" {
#include "log.h"
#include <libavformat/avformat.h>
}

PacketQueue::PacketQueue(int max_size) {
  queue_size_ = 0;
  max_size_ = max_size;
}

PacketQueue::~PacketQueue() { log_debug("PacketQueue Release!"); }


void PacketQueue::Push(void *pkt) {
     //log_debug("push befor mutex queue_size = %d",queue_size_);
   {
     std::unique_lock<std::mutex> lock(mutex_); 
     // log_debug("push befor wait queue_size = %d",queue_size_);
    // 使用单一条件变量，等待直到队列未满
     condition_.wait(lock, [this]() { return (queue_size_ < max_size_); });
    //    if (!condition_.wait_for(lock, std::chrono::milliseconds(100), [this]() { return queue_size_ < max_size_; })) {
    //     // 超时了，队列仍然满了
    //     log_debug("Push timed out, queue is still full.");
    // }

    // 执行 Push 操作

     packets_queue_.push(pkt);
    queue_size_++;
   // log_debug("push queue_size = %d", queue_size_.load());

    }

    // 唤醒等待 Pop 的线程
    condition_.notify_one();  // 因为同一个条件变量，要通知所有等待
}

void *PacketQueue::Pop() {
     //log_debug("pop befor mutex queue_size = %d",queue_size_);
    void *pkt = nullptr;
     {
std::unique_lock<std::mutex> lock(mutex_);
 //log_debug("pop befor wait queue_size = %d",queue_size_);
    // 使用单一条件变量，等待直到队列非空
    condition_.wait(lock, [this]() { return !packets_queue_.empty(); });

    // 执行 Pop 操作
    pkt = packets_queue_.front();
    packets_queue_.pop();
    queue_size_--;
   // log_debug("pop queue_size = %d", queue_size_.load());
     }
    

    // 唤醒等待 Push 的线程
    condition_.notify_one();  // 因为同一个条件变量，要通知所有等待
    return pkt;
}



// void PacketQueue::PushNullpacket() {
//   std::lock_guard<std::mutex> lock(mutex_);
//   void *pkt = av_packet_alloc();
//   av_init_packet(pkt);
//   packets_queue_.push(pkt);
//   queue_size_++;
//   condition_.notify_all();
// }
int PacketQueue::Size() {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_size_;
}
bool PacketQueue::Empty() {
  std::lock_guard<std::mutex> lock(mutex_);
  return packets_queue_.empty();
}
// void PacketQueue::Clear() {
//   std::lock_guard<std::mutex> lock(mutex_);
//   while (!packets_queue_.empty()) {
//     void *pkt = packets_queue_.front();
//     packets_queue_.pop();
//     av_packet_unref(pkt);
//     av_packet_free(&pkt);
//     queue_size_--;
//   }
// }

bool PacketQueue::Full()  {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_size_ == max_size_;
}
