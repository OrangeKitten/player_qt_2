#include "decodec.h"
extern "C" {
#include "log.h"
}

#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32-bit audio

static void sdl_audio_callback(void *opaque, Uint8 *stream, int len);

Decodec::Decodec() {
  dump_file_ = std::make_unique<FileDump>("audio.pcm");
  if (!(audio_frame_ = av_frame_alloc())) {
    log_error( "Could not allocate audio audio_frame_\n");
  }
  aduio_codec_info_ = (AVCodecParameters *)malloc(sizeof(AVCodecParameters));
  video_codec_info_ = (AVCodecParameters *)malloc(sizeof(AVCodecParameters));
  if(InitSDL()!=Ret_OK) {
    log_error( "init sdl failed");
  }


}

Ret Decodec::InitSDL() {
  sdl_flag_ = SDL_INIT_AUDIO ;
    if (SDL_Init (sdl_flag_)) {
        log_error( "Could not initialize SDL - %s\n", SDL_GetError());
        log_error( "(Did you set the DISPLAY variable?)\n");
        return Ret_ERROR;
    }
    //ToDo 目前写死
  want_audio_spec_.freq = 48000;
    want_audio_spec_.format = AUDIO_F32LSB;
    want_audio_spec_.channels = 2;
    want_audio_spec_.silence = 0;
    want_audio_spec_.samples = 1024;
    want_audio_spec_.callback = sdl_audio_callback;
  want_audio_spec_.userdata = this;
if ((audio_dev =SDL_OpenAudioDevice(nullptr,0,&want_audio_spec_, nullptr,SDL_AUDIO_ALLOW_FREQUENCY_CHANGE)) < 0)
    {
        log_error("can't open audio.\n");
        return Ret_ERROR;
    }
     SDL_PauseAudioDevice(audio_dev,0);
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
}
Ret Decodec::Init() {
  Ret ret = Ret_OK;
  audio_frame_queue_ = std::make_shared<PacketQueue>();
  video_frame_queue_ = std::make_shared<PacketQueue>();
  ret = InitAudio();
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
  video_decode_thread_ =
      std::make_unique<std::thread>(&Decodec::VideoThread, this);
}

void Decodec::VideoThread() {
  //     while (1) {
  //   AVPacket * video_pkt = video_pkt_queue_->Pop();
  //     dump_file_->Write(video_pkt, video_pkt->size);
  //      av_packet_free(&video_pkt);
  //  }
}

Ret Decodec::DecodeVideo(AVPacket *audio_pkt) { return Ret_OK; }
void Decodec::AudioThread() {
  while (1) {
    AVPacket *audio_pkt = (AVPacket*)audio_pkt_queue_->Pop();
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
  int samples_index = 0;
  int channel_index = 0;
  /* read all the output frames (in general there may be any number of them */
  while (ret >= 0) {
    //解析出来的是32bit float planar 小端数据
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

    // for (samples_index = 0; samples_index < audio_frame_->nb_samples; samples_index++)
    //   for (channel_index = 0; channel_index < audio_decodec_ctx_->channels;
    //        channel_index++)
    //     dump_file_->WriteData(
    //         audio_frame_->data[channel_index] + data_size * samples_index, data_size);
 
        // for (samples_index = 0;samples_index< audio_frame_->nb_samples; samples_index++)
        // {
        //     for (channel_index = 0; channel_index < audio_decodec_ctx_->channels; channel_index++)  // 交错的方式写入, 大部分float的格式输出
        //         fwrite(audio_frame_->data[channel_index] + data_size*samples_index, 1, data_size, outfile);
        // }
  }
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
  decodec_ = avcodec_find_decoder(aduio_codec_info_->codec_id);
  if (!decodec_) {
    log_error("Failed to find %s codec\n",
              av_get_media_type_string(aduio_codec_info_->codec_type));
    return Ret_ERROR;
  }

  /* Allocate a codec context for the decoder */
  audio_decodec_ctx_ = avcodec_alloc_context3(decodec_);
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
  if (avcodec_open2(audio_decodec_ctx_, decodec_, nullptr) < 0) {
    log_error("Failed to open %s codec\n",
              av_get_media_type_string(aduio_codec_info_->codec_type));
    return Ret_ERROR;
  }
}
Ret Decodec::InitVideo() { return Ret_OK; }


static void sdl_audio_callback(void *opaque, Uint8 *stream, int len) {
  Decodec * temp_decoec = (Decodec*)opaque;
    int len1, audio_size;

    static uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
    static unsigned int audio_buf_size = 0;
    static unsigned int audio_buf_index = 0;

    while (len > 0) {
        //通过while循环判断缓冲区大小是否大于0，如果len > 0，说明缓存区stream需要填充数据，那么就判断
        if (audio_buf_index >= audio_buf_size) {
            /* We have already sent all our data; get more */
            //audio缓冲区拷贝索引指针指向buf结尾，说明缓冲区所有数据都已拷贝到stream中，需要重新解码获取帧数据
      AVFrame *audio_frame = (AVFrame*)temp_decoec->audio_frame_queue_->Pop();

            audio_size = av_samples_get_buffer_size(nullptr,audio_frame->nb_samples,audio_frame->channels,(AVSampleFormat)audio_frame->format,0);

            if (audio_size < 0) {
                /* If error, output silence */
                audio_buf_size = 1024; // arbitrary?
                memset(audio_buf, 0, audio_buf_size);
            }
            else {
                //解码成功，则记录buf大小
                audio_buf_size = audio_size;
            }
            //重置buf索引为0
            audio_buf_index = 0;
        }
        //记录buf数据字节总数
        len1 = audio_buf_size - audio_buf_index;
        if (len1 > len)  //如果buf数据大于缓存区大小
            len1 = len; //先拷贝缓冲区大小的数据
        memcpy(stream, (uint8_t *)audio_buf + audio_buf_index, len1); //拷贝固定大小的数据
        len -= len1;  //记录下次需要拷贝的字节数
        stream += len1; //stream后移len1字节
        audio_buf_index += len1; //buf索引值后移len1字节
    }

  }
