#include "file_dump.h"
extern "C" {
#include "log.h"
}

const int sampling_frequencies[] = {
    96000, // 0x0
    88200, // 0x1
    64000, // 0x2
    48000, // 0x3
    44100, // 0x4
    32000, // 0x5
    24000, // 0x6
    22050, // 0x7
    16000, // 0x8
    12000, // 0x9
    11025, // 0xa
    8000   // 0xb
           // 0xc d e f是保留的
};
FileDump::FileDump(const char *filename) {

  file_ = fopen(filename, "wb+");
  if (!file_) {
    log_error("Error opening file");
  }
}
FileDump::~FileDump() {
  if (file_) {
    fclose(file_);
  }
}
void FileDump::WriteBitStream(const AVPacket *data, int size) {
  AVPacket *pkt = (AVPacket *)data;
#if 0 
    //在容器格式是mp4的时候，dump aac数据需要手动加包头
    if(format_ == AV_CODEC_ID_AAC) {
          char adts_header_buf[7] = {0};
            adts_header(adts_header_buf, audio_pkt->size,
                        1,
                        44100,
                        2);
            //printf("codecparID = %d\n",ifmt_ctx->streams[audio_index]->codecpar->codec_type);
            fwrite(adts_header_buf, 1, 7, file_);  // 写adts header , ts流不适用，ts流分离出来的packet带了adts header
            int len = fwrite( audio_pkt->data, 1, audio_pkt->size, file_);   // 写adts data
            if(len != audio_pkt->size)
            {
                log_error(NULL, AV_LOG_DEBUG, "warning, length of writed data isn't equal audio_pkt.size(%d, %d)\n",
                       len,
                       audio_pkt->size);
            }

    }
#endif
  int len = fwrite(pkt->data, 1, pkt->size, file_); // 写adts data
  if (len != pkt->size) {
    log_error(NULL, AV_LOG_DEBUG,
              "warning, length of writed data isn't equal pkt.size(%d, %d)\n",
              len, pkt->size);
  }
}

void FileDump::WriteData(const void *data, int size) {
#if 0 
    //在容器格式是mp4的时候，dump aac数据需要手动加包头
    if(format_ == AV_CODEC_ID_AAC) {
          char adts_header_buf[7] = {0};
            adts_header(adts_header_buf, audio_pkt->size,
                        1,
                        44100,
                        2);
            //printf("codecparID = %d\n",ifmt_ctx->streams[audio_index]->codecpar->codec_type);
            fwrite(adts_header_buf, 1, 7, file_);  // 写adts header , ts流不适用，ts流分离出来的packet带了adts header
            int len = fwrite( audio_pkt->data, 1, audio_pkt->size, file_);   // 写adts data
            if(len != audio_pkt->size)
            {
                log_error(NULL, AV_LOG_DEBUG, "warning, length of writed data isn't equal audio_pkt.size(%d, %d)\n",
                       len,
                       audio_pkt->size);
            }

    }
#endif
  int len = fwrite(data, 1, size, file_); 
  if (len !=size) {
    log_error(NULL, AV_LOG_DEBUG,
              "warning, length of writed data isn't equal pkt.size(%d, %d)\n",
              len,size);
  }
}

int FileDump::adts_header(char *const p_adts_header, const int data_length,
                          const int profile, const int samplerate,
                          const int channels) {

  int sampling_frequency_index = 3; // 默认使用48000hz
  int adtsLen = data_length + 7;

  int frequencies_size =
      sizeof(sampling_frequencies) / sizeof(sampling_frequencies[0]);
  int i = 0;
  for (i = 0; i < frequencies_size; i++) {
    if (sampling_frequencies[i] == samplerate) {
      sampling_frequency_index = i;
      break;
    }
  }
  if (i >= frequencies_size) {
    printf("unsupport samplerate:%d\n", samplerate);
    return -1;
  }

  p_adts_header[0] = 0xff; // syncword:0xfff                          高8bits
  p_adts_header[1] = 0xf0; // syncword:0xfff                          低4bits
  p_adts_header[1] |= (0 << 3); // MPEG Version:0 for MPEG-4,1 for MPEG-2  1bit
  p_adts_header[1] |= (0 << 1); // Layer:0                                 2bits
  p_adts_header[1] |= 1;        // protection absent:1                     1bit

  p_adts_header[2] = (profile) << 6; // profile:profile               2bits
  p_adts_header[2] |=
      (sampling_frequency_index & 0x0f)
      << 2; // sampling frequency index:sampling_frequency_index  4bits
  p_adts_header[2] |= (0 << 1); // private bit:0                   1bit
  p_adts_header[2] |=
      (channels & 0x04) >> 2; // channel configuration:channels  高1bit

  p_adts_header[3] = (channels & 0x03)
                     << 6;      // channel configuration:channels 低2bits
  p_adts_header[3] |= (0 << 5); // original：0                1bit
  p_adts_header[3] |= (0 << 4); // home：0                    1bit
  p_adts_header[3] |= (0 << 3); // copyright id bit：0        1bit
  p_adts_header[3] |= (0 << 2); // copyright id start：0      1bit
  p_adts_header[3] |= ((adtsLen & 0x1800) >> 11); // frame length：value 高2bits

  p_adts_header[4] =
      (uint8_t)((adtsLen & 0x7f8) >> 3); // frame length:value    中间8bits
  p_adts_header[5] =
      (uint8_t)((adtsLen & 0x7) << 5); // frame length:value    低3bits
  p_adts_header[5] |= 0x1f;            // buffer fullness:0x7ff 高5bits
  p_adts_header[6] =
      0xfc; // ‭11111100‬       //buffer fullness:0x7ff 低6bits
  // number_of_raw_data_blocks_in_frame：
  //    表示ADTS帧中有number_of_raw_data_blocks_in_frame + 1个AAC原始帧。

  return 0;
}
