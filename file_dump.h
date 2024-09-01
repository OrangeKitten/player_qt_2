#ifndef __FILE_DUMP_H__
#define __FILE_DUMP_H__
#include <cstdio>
extern "C" {
#include "libavcodec/avcodec.h"
}
class FileDump {
public:
  FileDump(const char *filename);
  ~FileDump();
  void WriteBitStream(const AVPacket *data, int size);
  void WriteData(const void *data, int size);

private:
  int adts_header(char *const p_adts_header, const int data_length,
                  const int profile, const int samplerate, const int channels);

private:
  FILE *file_;
  char filename_[128];
  AVCodecID audio_format_;
  AVCodecID video_format_;
  AVCodecID format_;

};
#endif