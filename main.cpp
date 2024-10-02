#include <iostream>
#include "video_player.h"
#include<string>
#include <thread>
#include <chrono>
 using namespace std;
//for SDL
#undef main
 int main() {
     char * url = "audio.mp4";
  VideoPlayer videoplay(url);
  videoplay.init();
  videoplay.Play();
 std::this_thread::sleep_for(std::chrono::seconds(30));
 printf("program finish ");
     return 0;
 }


