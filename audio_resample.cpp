#include "audio_resample.h"

extern "C" {
#include "log.h"
}

AudioReSample::AudioReSample() {
  swr_ctx_ = nullptr;
  //dump_file_ = std::make_unique<FileDump>("audio_resample.pcm");
  // resampled_data_ = nullptr;
  dst_nb_channels_ = 0;
}
AudioReSample::~AudioReSample() { // av_freep(&resampled_data_); 
}

  void AudioReSample::audio_resampler_alloc(
      const audio_resampler_params_t &resampler_params) {
    int ret = 0;
    // 初始化重采样
    swr_ctx_ = swr_alloc();
    if (!swr_ctx_) {
      log_error("swr_alloc failed\n");
      return;
    }
    /* set options */
    av_opt_set_sample_fmt(swr_ctx_, "in_sample_fmt",
                          resampler_params.src_sample_fmt, 0);
    av_opt_set_int(swr_ctx_, "in_channel_layout",
                   resampler_params.src_channel_layout, 0);
    av_opt_set_int(swr_ctx_, "in_sample_rate", resampler_params.src_sample_rate,
                   0);
    av_opt_set_sample_fmt(swr_ctx_, "out_sample_fmt",
                          resampler_params.dst_sample_fmt, 0);
    av_opt_set_int(swr_ctx_, "out_channel_layout",
                   resampler_params.dst_channel_layout, 0);
    av_opt_set_int(swr_ctx_, "out_sample_rate",
                   resampler_params.dst_sample_rate, 0);
    /* initialize the resampling context */
    ret = swr_init(swr_ctx_);
    if (ret < 0) {
      log_error("failed to initialize the resampling context.\n");
      swr_free(&swr_ctx_);
      return;
    }

    // 根据计算出的最大目标样本数，为目标样本数据分配内存，并返回一个指针数组
    // dst_data，以及每行数据的大小 dst_linesize
    ret = av_samples_alloc_array_and_samples(
        &resampled_data_, &dst_linesize_, resampler_params.dst_channel_number_,
        1024, resampler_params.dst_sample_fmt, 0);
    // 如果内存分配失败，打印错误信息，并跳转到 end 标签，释放资源并退出程序
    if (ret < 0) {
      log_error("Could not allocate destination samples\n");
      return;
    }
    printf("init done \n");
  }

  int AudioReSample::Init(int srcSampleRate, int srcChannelNumber,
                          AVSampleFormat srcFormat, int dstSampleRate,
                          int dstChannelNumber, AVSampleFormat dstFormat) {
    int ret = 0;

    if (srcSampleRate != dstSampleRate ||
        srcChannelNumber != dstChannelNumber || srcFormat != dstFormat) {
      // 重采样实例

      resampler_params_.src_channel_layout =
          av_get_default_channel_layout(srcChannelNumber);
      resampler_params_.src_sample_fmt = srcFormat;
      resampler_params_.src_sample_rate = srcSampleRate;
      resampler_params_.src_channel_number_ = srcChannelNumber;
      resampler_params_.dst_channel_layout =
          av_get_default_channel_layout(dstChannelNumber);
      resampler_params_.dst_sample_fmt = dstFormat;
      resampler_params_.dst_sample_rate = dstSampleRate;
      resampler_params_.dst_channel_number_ = dstChannelNumber;
      audio_resampler_alloc(resampler_params_);
      audio_fifo =
          av_audio_fifo_alloc(resampler_params_.dst_sample_fmt,
                              resampler_params_.dst_channel_number_, 1);
      if (!audio_fifo) {
        log_error("av_audio_fifo_alloc failed\n");
        return NULL;
      }
    }
  }
  // int AudioReSample::Resample(unsigned uint8_t *inData, int src_nb_sample,
  //                            unsigned uint8_t *outData, int outLen) {

  //  int nb_samples = swr_convert(swr_ctx_, resampled_data_,
  //  max_dst_nb_samples_,
  //                               (const uint8_t **)inData, src_nb_sample);
  //}

  int AudioReSample ::audio_resampler_send_frame(AVFrame * frame,
                                                 AVFrame * frame_resample) {
    if (frame == nullptr || frame_resample == nullptr) {
      log_error("frame or frame_resample == nullptr");
      return 0;
    }
    int src_nb_samples = 0;
    uint8_t **src_data = NULL;
    if (frame) {
      src_nb_samples = frame->nb_samples; // 输入源采样点数量
      src_data = frame->extended_data;    // 输入源的buffer
    }
    // 计算这次做重采样能够获取到的重采样后的点数
    const int dst_nb_samples = av_rescale_rnd(
        swr_get_delay(swr_ctx_, resampler_params_.src_sample_rate) +
            src_nb_samples,
        resampler_params_.src_sample_rate, resampler_params_.dst_sample_rate,
        AV_ROUND_UP);
    // if (dst_nb_samples > resampler->resampled_data_size)
    // {
    //     //resampled_data
    //     resampler->resampled_data_size = dst_nb_samples;
    //     if (init_resampled_data(resampler) < 0) {
    //         return AVERROR(ENOMEM);
    //     }
    // }
    int nb_samples =
        swr_convert(swr_ctx_, frame_resample->extended_data, dst_nb_samples,
                    (const uint8_t **)src_data, src_nb_samples);
    if (nb_samples != dst_nb_samples) {
      log_error(" nb_sample != dst_nb_samples");
      return 0;
    }
    return nb_samples;

    // log_debug("nb_samples = %d",nb_samples);
    //  返回实际写入的采样点数量
    //     return av_audio_fifo_write(resampler->audio_fifo, (void
    //     **)resampler->resampled_data, nb_samples);
    // int data_size =
    // av_get_bytes_per_sample(resampler_params_.dst_sample_fmt);
    // dump_file_->WritePcmData(resampled_data_,
    //                                dst_nb_samples * data_size * 2);

    // int ret_size = av_audio_fifo_write(
    //     audio_fifo, (void **)resampled_data_, nb_samples);
    // if (ret_size != dst_nb_samples) {
    //   log_warn("Warn:av_audio_fifo_write failed, expected_write:%d, "
    //          "actual_write:%d\n",
    //          nb_samples, ret_size);
    // }
  }
