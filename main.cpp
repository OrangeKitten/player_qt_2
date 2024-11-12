#include <iostream>
#include "video_player.h"
#include<string>
#include <thread>
#include <chrono>
extern "C"
{
#include "log.h"
}
 using namespace std;
//for SDL
#undef main
 int main() {

   //char * url = "Sync-One2_Test_1080p_23.98_H.264_AAC_5.1.ts";
     char * url = "audio.mp4";
    // char * url = "source.200kbps.768x320_48.flv";
    // char * url ="output.mp4";
  VideoPlayer videoplay(url);
  videoplay.init();
  videoplay.Play();
std::this_thread::sleep_for(std::chrono::seconds(10));
videoplay.Pause();
std::this_thread::sleep_for(std::chrono::seconds(10));

videoplay.Pause();
// videoplay.set_mute(true);
//  std::this_thread::sleep_for(std::chrono::seconds(2));
//  videoplay.set_mute(false);
// std::this_thread::sleep_for(std::chrono::seconds(5));
// videoplay.set_volume(0);
 printf("program finish ");

     return 0;
 }
