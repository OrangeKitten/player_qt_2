
#ifndef __DEMUX_H__
#define __DEMUX_H__
#include <string>
#include "packet_queue.h"
#include <memory>
#include <thread>
#include "type.h"
#include "file_dump.h"

extern "C"
{
#include <libavformat/avformat.h>
}
class Demux
{
public:
    Demux(char *url);
    ~Demux();
    void setAudioQueue(std::shared_ptr<PacketQueue>pkt_queue);
    void setVideoQueue(std::shared_ptr<PacketQueue>pkt_queue);
    void StartReadPacket();
    Ret getAudioAvCodecInfo( AVCodecParameters *dec);
    Ret getVideoAvCodecInfo( AVCodecParameters *dec);
private:
    void DumpMedioInfo();
    int OpenFile();
    void ReadPacketThread();


private:
    AVFormatContext *format_ctx_;
    AVIOContext *avio_ctx_;
    AVStream *video_stream_;
    AVStream *audio_stream_;
    int video_stream_index_;
    int audio_stream_index_;
    char *url_;
    int pkt_count_;
    std::shared_ptr<PacketQueue>audio_pkt_queue_;
    std::shared_ptr<PacketQueue>video_pkt_queue_;
    std::unique_ptr<std::thread> read_packet_thread_;
    int read_size_;
    int write_size_;
     std::unique_ptr<FileDump>  dump_file_;

};
#endif // DEMUX_H
