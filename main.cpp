#if 1
#include <iostream>
#include "video_player.h"
#include<string>
#include <thread>
#include <chrono>
 using namespace std;
//for SDL
#undef main
 int main() {
     char * url = "audio48hz.mp4";
  VideoPlayer videoplay(url);
  videoplay.init();
  videoplay.Play();
 std::this_thread::sleep_for(std::chrono::seconds(30));
 printf("program finish ");
     return 0;
 }

#else if
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "SDL.h"
};

// Forward declaration
void cleanup(SDL_Window *window, SDL_Renderer *renderer, SDL_Texture *texture, AVCodecContext *pCodecContext, AVFormatContext *pFormatContext);
#undef main
int main(int argc, char *argv[]) {

    const char *file_path = "audio48hz.mp4";


    // Open the input video file
    AVFormatContext *pFormatContext = avformat_alloc_context();
    if (avformat_open_input(&pFormatContext, file_path, NULL, NULL) != 0) {
        printf("Failed to open video file\n");
        return -1;
    }

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatContext, NULL) < 0) {
        printf("Failed to retrieve input stream information\n");
        avformat_close_input(&pFormatContext);
        return -1;
    }

    // Find the video stream index
    int videoStreamIndex = -1;
    for (int i = 0; i < pFormatContext->nb_streams; i++) {
        if (pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }
    if (videoStreamIndex == -1) {
        printf("Failed to find a video stream\n");
        avformat_close_input(&pFormatContext);
        return -1;
    }

    // Get the codec parameters and find the decoder
    AVCodecParameters *pCodecParameters = pFormatContext->streams[videoStreamIndex]->codecpar;
    AVCodec *pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
    if (pCodec == NULL) {
        printf("Failed to find a codec\n");
        avformat_close_input(&pFormatContext);
        return -1;
    }

    // Allocate codec context and open codec
    AVCodecContext *pCodecContext = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecContext, pCodecParameters);
    if (avcodec_open2(pCodecContext, pCodec, NULL) < 0) {
        printf("Failed to open codec\n");
        avcodec_free_context(&pCodecContext);
        avformat_close_input(&pFormatContext);
        return -1;
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        avcodec_free_context(&pCodecContext);
        avformat_close_input(&pFormatContext);
        return -1;
    }

    // Create an SDL window
    SDL_Window *window = SDL_CreateWindow(
        "FFmpeg & SDL Video Player",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        pCodecContext->width, pCodecContext->height,
        SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Failed to create SDL window: %s\n", SDL_GetError());
        cleanup(window, NULL, NULL, pCodecContext, pFormatContext);
        return -1;
    }

    // Create an SDL renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        printf("Failed to create SDL renderer: %s\n", SDL_GetError());
        cleanup(window, renderer, NULL, pCodecContext, pFormatContext);
        return -1;
    }

    // Create an SDL texture
    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_IYUV,
        SDL_TEXTUREACCESS_STREAMING,
        pCodecContext->width,
        pCodecContext->height);
    if (!texture) {
        printf("Failed to create SDL texture: %s\n", SDL_GetError());
        cleanup(window, renderer, texture, pCodecContext, pFormatContext);
        return -1;
    }

    // Allocate AVFrame for decoded video frame and YUV frame
    AVFrame *pFrame = av_frame_alloc();
//    AVFrame *pFrameYUV = av_frame_alloc();
//    uint8_t *buffer = (uint8_t *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecContext->width, pCodecContext->height, 1));
//    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, buffer, AV_PIX_FMT_YUV420P, pCodecContext->width, pCodecContext->height, 1);

    // Set up software scaling context for conversion from native format to YUV420P
//    struct SwsContext *sws_ctx = sws_getContext(
//        pCodecContext->width, pCodecContext->height, pCodecContext->pix_fmt,
//        pCodecContext->width, pCodecContext->height, AV_PIX_FMT_YUV420P,
//        SWS_BILINEAR, NULL, NULL, NULL);

    AVPacket *packet = av_packet_alloc();
    int response;

    // Main decoding loop
    while (av_read_frame(pFormatContext, packet) >= 0) {
        if (packet->stream_index == videoStreamIndex) {
            response = avcodec_send_packet(pCodecContext, packet);
            if (response < 0) {
                printf("Failed to send packet to decoder\n");
                continue;
            }

            while (response >= 0) {
                response = avcodec_receive_frame(pCodecContext, pFrame);
                if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                    break;
                } else if (response < 0) {
                    printf("Failed to receive frame from decoder\n");
                    return response;
                }

                // Convert the frame to YUV420P format for SDL
               // sws_scale(sws_ctx, (const uint8_t *const *)pFrame->data, pFrame->linesize, 0, pCodecContext->height, pFrameYUV->data, pFrameYUV->linesize);

                // Update texture with the new frame
                SDL_UpdateYUVTexture(texture, NULL, pFrame->data[0], pFrame->linesize[0], pFrame->data[1], pFrame->linesize[1], pFrame->data[2], pFrame->linesize[2]);

                // Render frame
                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, texture, NULL, NULL);
                SDL_RenderPresent(renderer);
                 SDL_Delay(40); // 控制帧率大约为 25fps (1000ms / 25 = 40ms)
                SDL_Event event;
                SDL_PollEvent(&event);
                if (event.type == SDL_QUIT) {
                    av_packet_unref(packet);
                    goto cleanup;
                }
            }
        }
        av_packet_unref(packet);
    }

cleanup:
    // Clean up resources
    cleanup(window, renderer, texture, pCodecContext, pFormatContext);
    //av_free(buffer);
    av_frame_free(&pFrame);
    //av_frame_free(&pFrameYUV);
    av_packet_free(&packet);
    //sws_freeContext(sws_ctx);

    return 0;
}

void cleanup(SDL_Window *window, SDL_Renderer *renderer, SDL_Texture *texture, AVCodecContext *pCodecContext, AVFormatContext *pFormatContext) {
    if (texture) SDL_DestroyTexture(texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    if (pCodecContext) avcodec_free_context(&pCodecContext);
    if (pFormatContext) avformat_close_input(&pFormatContext);
    SDL_Quit();
}


#endif
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
