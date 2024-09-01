 #include <iostream>
#include "video_player.h"
#include<string>
#include <thread>
#include <chrono>
 using namespace std;
//for SDL
#undef main
 int main() {
     char * url = "believe.ts";
  VideoPlayer videoplay(url);
  videoplay.init();
  videoplay.Play();
 std::this_thread::sleep_for(std::chrono::seconds(10));
 printf("program finish ");
     return 0;
 }


