#include <iostream>
#include "video_player.h"
#include<string>
#include <thread>
#include <chrono>
 using namespace std;
//for SDL
#undef main
 int main() {
   //char * url = "audio.mp4";
     //char * url = "audio48hz.mp4";
     char * url = "source.200kbps.768x320_48.flv";
  VideoPlayer videoplay(url);
  videoplay.init();
  videoplay.Play();
 std::this_thread::sleep_for(std::chrono::seconds(30));
 printf("program finish ");
     return 0;
 }

//#include<vector>
//#include<ringbuf.h>
//int main() {
//    RingBuffer ring(6); // Create a ring buffer with a capacity of 10 bytes

//    // Push some data into the ring buffer
//    ring.push({'H', 'e', 'l', 'l', 'o'});

//    // Pop data from the ring buffer
//    std::vector<char> data = ring.pop(5);
//    std::cout << std::string(data.begin(), data.end()) << std::endl; // Output: Hello
//         ring.push({'d','a','y'});
//     data = ring.pop(5);

//       std::cout << std::string(data.begin(), data.end()) << std::endl; // Output: Hello
//    return 0;
//}
