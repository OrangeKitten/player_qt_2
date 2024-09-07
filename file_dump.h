#ifndef __FILE_DUMP_H__
#define __FILE_DUMP_H__
#include "type.h"
#include <cstdio>

extern "C" {
#include "libavcodec/avcodec.h"
}
void setAudioInfo(const Auido_Info &audio_info);
class FileDump {
public:
  FileDump(const char *filename);
  ~FileDump();
  void WriteBitStream(const AVPacket *data, int size);
  void WritePcmData(uint8_t *data[], int size_totol_sample);
  void WritePcmPlanarData(uint8_t *data[], int size_per_sample);

private:
  int adts_header(char *const p_adts_header, const int data_length,
                  const int profile, const int samplerate, const int channels);
  bool is_audio_format_planar();

private:
  FILE *file_;
  char filename_[128];
  AVCodecID audio_format_;
  AVCodecID video_format_;
  AVCodecID format_;
};
#endif
