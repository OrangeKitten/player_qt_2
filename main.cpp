 #include <iostream>
#include "video_player.h"
#include<string>
#include <thread>
#include <chrono>
 using namespace std;

 int main() {
     char * url = "2_audio.mp4";
  VideoPlayer videoplay(url);
  videoplay.init();
  videoplay.Play();
 std::this_thread::sleep_for(std::chrono::seconds(1));
     return 0;
 }

