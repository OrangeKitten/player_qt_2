#ifndef __DECODEC_H__
#define __DECODEC_H__
#include "packet_queue.h"
#include "type.h"
#include <memory>
#include <thread>
#include "file_dump.h"
#include "audio_resample.h"
#include <SDL.h>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

}
class Decodec {
public:
  Decodec();
  ~Decodec();
  Ret setVolume(int volume);
  void setAudioQueue(std::shared_ptr<PacketQueue> pkt_queue);
  void setVideoQueue(std::shared_ptr<PacketQueue> pkt_queue);
  Ret Init();
  void StartDecode();
  Ret setAudioAvCodecInfo(AVCodecParameters *dec);
  Ret setVideoAvCodecInfo(AVCodecParameters *dec);
  void DumpAudioCodecInfo();
  void DumpvideoCodecInfo();

private:
  void AudioThread();
  void VideoThread();
  Ret InitAudio();
  Ret InitVideo();
  Ret DecodeAudio(AVPacket*audio_pkt);
  Ret DecodeVideo(AVPacket*audio_pkt);
 Ret InitSDL();
 AVFrame * AllocOutFrame();
 AVSampleFormat TransforSDLFormattoFFmpeg(int SDL_format);

private:
  std::shared_ptr<PacketQueue> audio_pkt_queue_;
  std::shared_ptr<PacketQueue> video_pkt_queue_;
  std::unique_ptr<std::thread> audio_decode_thread_;
  std::unique_ptr<std::thread> video_decode_thread_;
  AVCodecParameters * aduio_codec_info_;
  AVCodecParameters * video_codec_info_;
 
  AVCodecContext * audio_decodec_ctx_;
  AVCodec *decodec_;
  AVFrame *audio_frame_;
  AVFrame *audio_frame_resample_;
  int sdl_flag_;
  SDL_AudioSpec want_audio_spec_;
  SDL_AudioDeviceID audio_dev;
  std::unique_ptr<AudioReSample> audio_resample_;
public:
  std::shared_ptr<PacketQueue> audio_frame_queue_;
  std::shared_ptr<PacketQueue> video_frame_queue_;
  std::unique_ptr<FileDump>  dump_file_;


};
#endif
