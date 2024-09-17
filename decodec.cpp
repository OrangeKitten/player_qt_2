#include "decodec.h"
extern "C" {
#include "log.h"
}

#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32-bit audio

static void sdl_audio_callback(void *opaque, Uint8 *stream, int len);
#define AUDIOFRAME 8
#define VIDEOFRAME 3
Decodec::Decodec() {
  dump_file_ = std::make_unique<FileDump>("audio.pcm");
  
  if (!(audio_frame_ = av_frame_alloc())) {
    log_error("Could not allocate audio audio_frame_\n");
  }
  if (!(video_frame_ = av_frame_alloc())) {
    log_error("Could not allocate audio video_frame_\n");
  }

  aduio_codec_info_ = (AVCodecParameters *)malloc(sizeof(AVCodecParameters));
  video_codec_info_ = (AVCodecParameters *)malloc(sizeof(AVCodecParameters));

  audio_decodec_ctx_ = nullptr;
  video_decodec_ctx_ = nullptr;
  audio_decodec_ = nullptr;
  video_decodec_ = nullptr;
  audio_frame_resample_ = nullptr;

  if (InitSDL() != Ret_OK) {
    log_error("init sdl failed");
  }
  audio_resample_ = std::make_unique<AudioReSample>();
}

Ret Decodec::InitSDL() {
  sdl_flag_ = SDL_INIT_AUDIO;
  if (SDL_Init(sdl_flag_) < 0) {
    log_error("Could not initialize SDL - %s\n", SDL_GetError());
    log_error("(Did you set the DISPLAY variable?)\n");
    return Ret_ERROR;
  }
  // ToDo 目前写死
  want_audio_spec_.freq = 48000;
  // want_audio_spec_.format = AV_SAMPLE_FMT_S16; // ffmpeg定义
  want_audio_spec_.format = AUDIO_S16LSB; // SDL 定义
  want_audio_spec_.channels = 2;
  want_audio_spec_.silence = 0;
  want_audio_spec_.samples = 1024;
  want_audio_spec_.callback = sdl_audio_callback;
  want_audio_spec_.userdata = this;
  int num = SDL_GetNumAudioDevices(0);
  const char *deviceName = SDL_GetAudioDeviceName(num, 0);
  if ((audio_dev =
           SDL_OpenAudioDevice(deviceName, 0, &want_audio_spec_, nullptr,
                               SDL_AUDIO_ALLOW_FREQUENCY_CHANGE)) < 2) {
    log_error("can't open audio.\n");
    return Ret_ERROR;
  }
  SDL_PauseAudioDevice(audio_dev, 0);
  return Ret_OK;
}
Decodec::~Decodec() {
  if (audio_decode_thread_->joinable()) {
    audio_decode_thread_->join();
  }
  if (video_decode_thread_->joinable()) {
    video_decode_thread_->join();
  }

  free(aduio_codec_info_);
  free(video_codec_info_);
  av_frame_free(&audio_frame_);
  // av_frame_free(&audio_frame_resample_);
}
Ret Decodec::Init() {
  Ret ret = Ret_OK;
  audio_frame_queue_ = std::make_shared<PacketQueue>(AUDIOFRAME);
  video_frame_queue_ = std::make_shared<PacketQueue>(VIDEOFRAME);
  ret = InitAudio();
  ret = InitVideo();
  audio_resample_->Init(
      aduio_codec_info_->sample_rate,
      av_get_channel_layout_nb_channels(aduio_codec_info_->channel_layout),
      (AVSampleFormat)aduio_codec_info_->format, want_audio_spec_.freq,
      want_audio_spec_.channels,
      TransforSDLFormattoFFmpeg((AVSampleFormat)want_audio_spec_.format));
  return ret;
}
void Decodec::setAudioQueue(std::shared_ptr<PacketQueue> pkt_queue) {
  audio_pkt_queue_ = std::move(pkt_queue);
}
void Decodec::setVideoQueue(std::shared_ptr<PacketQueue> pkt_queue) {
  video_pkt_queue_ = std::move(pkt_queue);
}
Ret Decodec::setVolume(int volume) { return Ret_OK; }

void Decodec::StartDecode() {

  DumpAudioCodecInfo();
  audio_decode_thread_ =
      std::make_unique<std::thread>(&Decodec::AudioThread, this);
  // video_decode_thread_ =
  //     std::make_unique<std::thread>(&Decodec::VideoThread, this);
}

void Decodec::VideoThread() {
  while (1) {
    AVPacket *video_pkt = (AVPacket *)video_pkt_queue_->Pop();
    DecodeAudio(video_pkt);
    av_packet_free(&video_pkt);
  }
}

Ret Decodec::DecodeVideo(AVPacket *video_pkt) {
  int ret = avcodec_send_packet(video_decodec_ctx_, video_pkt);
  if (ret < 0) {
    log_error("Failed to send packet to decoder\n");
    return Ret_ERROR;
  }
  int data_size = 0;
  int channel_index = 0;
  /* read all the output frames (in general there may be any number of them */
  while (ret >= 0) {
    // 解析出来的是32bit float planar 小端数据
    ret = avcodec_receive_frame(video_decodec_ctx_, video_frame_);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
      return Ret_ERROR;
    else if (ret < 0) {
      log_error("Error during decoding\n");
      return Ret_ERROR;
    }
    dump_file_->WritePcmData(video_frame_->data, data_size);
    audio_frame_queue_->Push(audio_frame_resample_);
  }
}
void Decodec::AudioThread() {
  while (1) {
    AVPacket *audio_pkt = (AVPacket *)audio_pkt_queue_->Pop();
    // dump_file_->WriteData(audio_pkt->data, audio_pkt->size);
    DecodeAudio(audio_pkt);
    av_packet_free(&audio_pkt);
  }
}

Ret Decodec::DecodeAudio(AVPacket *audio_pkt) {
  int ret = avcodec_send_packet(audio_decodec_ctx_, audio_pkt);
  if (ret < 0) {
    log_error("Failed to send packet to decoder\n");
    return Ret_ERROR;
  }
  int data_size = 0;

  int channel_index = 0;
  /* read all the output frames (in general there may be any number of them */
  while (ret >= 0) {
    // 解析出来的是32bit float planar 小端数据
    ret = avcodec_receive_frame(audio_decodec_ctx_, audio_frame_);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
      return Ret_ERROR;
    else if (ret < 0) {
      log_error("Error during decoding\n");
      return Ret_ERROR;
    }
    data_size = av_get_bytes_per_sample(audio_decodec_ctx_->sample_fmt);
    if (data_size < 0) {
      /* This should not occur, checking just for paranoia */
      log_error("Failed to calculate data size\n");
      return Ret_ERROR;
    }

    // dump_file_->WritePcmData(audio_frame_->data, data_size);
    if (!(audio_frame_resample_ = AllocOutFrame())) {
      log_error("Could not allocate audio audio_frame_resample_\n");
    }
    // audio_frame_resample_是resample之后的数据，
    int resample_sample_number = audio_resample_->audio_resampler_send_frame(
        audio_frame_, audio_frame_resample_);
    // dump_file_->WritePcmData(audio_frame_resample_->extended_data,
    //                          resample_sample_number * 2 * 2);
    audio_frame_queue_->Push(audio_frame_resample_);
  }
}
AVFrame *Decodec::AllocOutFrame() {
  int ret;
  AVFrame *frame = av_frame_alloc();
  if (!frame) {
    return NULL;
  }
  frame->nb_samples = want_audio_spec_.samples;
  frame->channel_layout =
      av_get_default_channel_layout(want_audio_spec_.channels);
  frame->format = TransforSDLFormattoFFmpeg(want_audio_spec_.format);

  frame->sample_rate = want_audio_spec_.freq;
  ret = av_frame_get_buffer(frame, 0);
  if (ret < 0) {
    printf("cannot allocate audio data buffer\n");
    return NULL;
  }
  return frame;
}

Ret Decodec::setAudioAvCodecInfo(AVCodecParameters *dec) {
  if (aduio_codec_info_ == nullptr && dec == nullptr) {
    return Ret_ERROR;
  }
  memcpy(aduio_codec_info_, dec, sizeof(AVCodecParameters));
  return Ret_OK;
}
Ret Decodec::setVideoAvCodecInfo(AVCodecParameters *dec) {
  if (video_codec_info_ == nullptr && dec == nullptr) {
    return Ret_ERROR;
  }
  memcpy(video_codec_info_, dec, sizeof(AVCodecParameters));
  return Ret_OK;
}

void Decodec::DumpAudioCodecInfo() {
  log_debug("\n----- Audio info:\n");
  // sample_rate: 音频编解码器的采样率，单位为Hz
  log_debug("samplerate:%dHz\n", aduio_codec_info_->sample_rate);
  // channels: 音频通道数，单声道为1，双声道为2，5.1声道为6
  log_debug("channels:%d\n", aduio_codec_info_->channels);
  // channel_layout:
  // 音频通道布局，用于描述音频通道的布局，例如立体声为2个通道，左声道为1，右声道为2
  log_debug("channel_layout:%d\n", aduio_codec_info_->channel_layout);
  // codec_type: 音频编解码器的类型，通常为AVMEDIA_TYPE_AUDIO
  log_debug("codec_type:%d\n", aduio_codec_info_->codec_type);
}

void Decodec::DumpvideoCodecInfo() {
  log_debug("\n----- Video info:\n");
  // index: 每个流成分在ffmpeg解复用分析后都有唯一的index作为标识
}

Ret Decodec::InitAudio() {
  /* find decoder for the stream */
  audio_decodec_ = avcodec_find_decoder(aduio_codec_info_->codec_id);
  if (!audio_decodec_) {
    log_error("Failed to find %s codec\n",
              av_get_media_type_string(aduio_codec_info_->codec_type));
    return Ret_ERROR;
  }

  /* Allocate a codec context for the decoder */
  audio_decodec_ctx_ = avcodec_alloc_context3(audio_decodec_);
  if (!audio_decodec_ctx_) {
    log_error("Failed to allocate the %s codec context\n",
              av_get_media_type_string(aduio_codec_info_->codec_type));
    return Ret_ERROR;
  }
  /* Copy codec parameters from input stream to output codec context */
  if (avcodec_parameters_to_context(audio_decodec_ctx_, aduio_codec_info_) <
      0) {
    log_error("Failed to copy %s codec parameters to decoder context\n",
              av_get_media_type_string(aduio_codec_info_->codec_type));
    return Ret_ERROR;
  }
  /* Init the decoders, with or without reference counting */
  // av_dict_set(&opts, "refcounted_frames", refcount ? "1" : "0", 0);
  if (avcodec_open2(audio_decodec_ctx_, audio_decodec_, nullptr) < 0) {
    log_error("Failed to open %s codec\n",
              av_get_media_type_string(aduio_codec_info_->codec_type));
    return Ret_ERROR;
  }
}
Ret Decodec::InitVideo() {
  /* find decoder for the stream */
  video_decodec_ = avcodec_find_decoder(video_codec_info_->codec_id);
  if (!video_decodec_) {
    log_error("Failed to find %s codec\n",
              av_get_media_type_string(video_codec_info_->codec_type));
    return Ret_ERROR;
  }

  /* Allocate a codec context for the decoder */
  video_decodec_ctx_ = avcodec_alloc_context3(video_decodec_);
  if (!video_decodec_ctx_) {
    log_error("Failed to allocate the %s codec context\n",
              av_get_media_type_string(video_codec_info_->codec_type));
    return Ret_ERROR;
  }
  /* Copy codec parameters from input stream to output codec context */
  if (avcodec_parameters_to_context(video_decodec_ctx_, video_codec_info_) <
      0) {
    log_error("Failed to copy %s codec parameters to decoder context\n",
              av_get_media_type_string(video_codec_info_->codec_type));
    return Ret_ERROR;
  }
  /* Init the decoders, with or without reference counting */
  // av_dict_set(&opts, "refcounted_frames", refcount ? "1" : "0", 0);
  if (avcodec_open2(video_decodec_ctx_, video_decodec_, nullptr) < 0) {
    log_error("Failed to open %s codec\n",
              av_get_media_type_string(video_codec_info_->codec_type));
    return Ret_ERROR;
  }
}

static void sdl_audio_callback(void *opaque, Uint8 *stream, int len) {

  // TODO
  // 创建一个缓存buff来存储队列中的解码数据，并且是在while
  // 循环中来做Pop的动作，然后把本次回调没有读取的数据存入到缓存中方便下次读取
  Decodec *temp_decoec = (Decodec *)opaque;
  int len1, audio_size;

  // static uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
  uint8_t *audio_buf = nullptr;
  static unsigned int audio_buf_size = 0;
  static unsigned int audio_buf_index = 0;
  AVFrame *audio_frame = (AVFrame *)temp_decoec->audio_frame_queue_->Pop();
  audio_buf = audio_frame->data[0];
  // temp_decoec->dump_file_->WritePcmData(audio_frame->extended_data,audio_frame->nb_samples
  // * 2 * 2);

  audio_size = av_samples_get_buffer_size(
      nullptr, audio_frame->channels, audio_frame->nb_samples,
      (AVSampleFormat)audio_frame->format, 0);
  log_debug("audio_size:%d\n", audio_size);
  //   log_debug("audio_frame->nb_samples:%d\n",audio_frame->nb_samples);
  //   log_debug("audio_frame->channels:%d\n",audio_frame->channels);
  //   log_debug("audio_frame->format:%d\n",audio_frame->format);

  while (len > 0) {
    // 通过while循环判断缓冲区大小是否大于0，如果len >
    // 0，说明缓存区stream需要填充数据，那么就判断
    if (audio_buf_index >= audio_buf_size) {
      /* We have already sent all our data; get more */
      //
      // audio缓冲区拷贝索引指针指向buf结尾，说明缓冲区所有数据都已拷贝到stream中，需要重新解码获取帧数据

      // audio_size = audio_frame->nb_samples *audio_frame->channels*2;

      if (audio_size < 0) {
        /* If error, output silence */
        audio_buf_size = 1024; // arbitrary?
        memset(audio_buf, 0, audio_buf_size);
      } else {
        // 解码成功，则记录buf大小
        audio_buf_size = audio_size;
      }
      // 重置buf索引为0
      audio_buf_index = 0;
    }
    // 记录buf数据字节总数
    len1 = audio_buf_size - audio_buf_index;
    if (len1 > len) { // 如果buf数据大于缓存区大小
      len1 = len;     // 先拷贝缓冲区大小的数据
    }
    memcpy(stream, (uint8_t *)audio_buf + audio_buf_index,
           len1);            // 拷贝固定大小的数据
    len -= len1;             // 记录下次需要拷贝的字节数
    stream += len1;          // stream后移len1字节
    audio_buf_index += len1; // buf索引值后移len1字节
  }
}

AVSampleFormat Decodec::TransforSDLFormattoFFmpeg(int SDL_format) {
  switch (SDL_format) {
  case AUDIO_S16SYS:
    return AV_SAMPLE_FMT_S16;
  case AUDIO_S32SYS:
    return AV_SAMPLE_FMT_S32;
  case AUDIO_S16MSB:
    return AV_SAMPLE_FMT_S16;
  case AUDIO_U8:
    return AV_SAMPLE_FMT_U8;
  default:
    return AV_SAMPLE_FMT_NONE;
  }
}
