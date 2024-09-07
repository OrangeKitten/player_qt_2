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


//#include <SDL.h>
//#include <iostream>
//#include <fstream>
//#include <thread>
//#include <chrono>
//#undef main
//// 播放器数据结构
//struct AudioData {
//    Uint8* pos;    // 当前音频数据位置
//    Uint32 length; // 剩余音频数据长度
//};
//using namespace std;
//// 回调函数，用于提供音频数据
//void AudioCallback(void* userdata, Uint8* stream, int len) {
//    AudioData* audio = (AudioData*)userdata;
//    if (audio->length == 0) {
//        return;
//    }
//    len = (len > audio->length ? audio->length : len);
//    SDL_memcpy(stream, audio->pos, len);
//    audio->pos += len;
//    audio->length -= len;
//}
//int main(int argc, char* argv[]) {
//    // 初始化 SDL
//    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
//        std::cerr << "Could not initialize SDL: " << SDL_GetError() << std::endl;
//        return -1;
//    }
//    // 打开 PCM 文件
//    std::ifstream pcmFile("output.pcm", std::ios::binary);
//    if (!pcmFile) {
//        std::cerr << "Could not open PCM file." << std::endl;
//        SDL_Quit();
//        return -1;
//    }
//    // 获取 PCM 文件大小
//    pcmFile.seekg(0, std::ios::end);
//    Uint32 pcmLength = pcmFile.tellg();
//    pcmFile.seekg(0, std::ios::beg);
//    // 分配缓冲区
//    Uint8* pcmBuffer = new Uint8[pcmLength];
//    pcmFile.read(reinterpret_cast<char*>(pcmBuffer), pcmLength);
//    pcmFile.close();
//    // 设置音频规格
//    SDL_AudioSpec wavSpec;
//    wavSpec.freq = 48000;               // 采样率
//    wavSpec.format = AUDIO_S16LSB;      // 音频格式 (16-bit little-endian)
//    wavSpec.channels = 2;               // 通道数量 (立体声)
//    wavSpec.silence = 0;
//    wavSpec.samples = 4096;             // 缓冲区大小
//    wavSpec.callback = AudioCallback;   // 回调函数
//    wavSpec.userdata = new AudioData{ pcmBuffer, pcmLength };
//    cout<<"Open Audio Device"<<endl;
//    // 打开音频设备
////    if (SDL_OpenAudio(&wavSpec, NULL) < 0) {
////        std::cerr << "Could not open audio device: " << SDL_GetError() << std::endl;
////        delete[] pcmBuffer;
////        SDL_Quit();
////        return -1;
////    }
////    // 开始播放音频
////    SDL_PauseAudio(0);
//    int num = SDL_GetNumAudioDevices(0);
//    SDL_AudioDeviceID audio_dev;
//      const char* deviceName = SDL_GetAudioDeviceName(num, 0);
//    if ((audio_dev = SDL_OpenAudioDevice(deviceName, 0, &wavSpec, nullptr,
//                                         SDL_AUDIO_ALLOW_FREQUENCY_CHANGE)) < 2) {
//      printf("can't open audio.\n");
//     return -1;
//    }
//    SDL_PauseAudioDevice(audio_dev, 0);
//    // 等待音频播放结束
//    while (reinterpret_cast<AudioData*>(wavSpec.userdata)->length > 0) {
//        SDL_Delay(100); // 轮询，等待音频播放完毕
//    }
// cout<<"Close Audio Device"<<endl;
//    // 关闭音频设备并清理
//    SDL_CloseAudio();
//    delete[] pcmBuffer;
//    delete reinterpret_cast<AudioData*>(wavSpec.userdata);
//    SDL_Quit();
//this_thread::sleep_for(chrono::seconds(10));
//    return 0;
//}


//#include <stdio.h>
//extern "C" {
//#include <libavutil/samplefmt.h>
//}

//int main() {
//    int linesize, bufsize;
//    int nb_channels = 2;
//    int nb_samples = 1024;
//    enum AVSampleFormat format = AV_SAMPLE_FMT_S16;
//    int align = 1;
//    bufsize = av_samples_get_buffer_size(&linesize, nb_channels, nb_samples, format, align);

//    if (bufsize < 0) {
//        printf("Error: Unable to calculate buffer size\n");
//        return -1;
//    }

//    printf("Buffer size for %d channels, %d samples, and %s format is %d bytes\n",
//           nb_channels, nb_samples, av_get_sample_fmt_name(format), bufsize);

//    return 0;
//}
