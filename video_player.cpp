#include "video_player.h"
extern "C" {
#include "log.h"
}
VideoPlayer::VideoPlayer (char *url)
{
     play_ = false;
     pause_ = false;
     position_ =-1;
     volume_ =-1;
     url_ = url;
    
}

Ret  VideoPlayer::init(){
demux_ = std::make_unique<Demux>(url_);
decodec_ = std::make_unique<Decodec>();

AVCodecParameters *audio_dec = (AVCodecParameters*)malloc(sizeof(AVCodecParameters));
AVCodecParameters *video_dec = (AVCodecParameters*)malloc(sizeof(AVCodecParameters));
demux_->getAudioAvCodecInfo(audio_dec);
demux_->getVideoAvCodecInfo(video_dec);
//创建AvpacketQueue
audio_pkt_queue_ = std::make_shared<PacketQueue>();
video_pkt_queue_ = std::make_shared<PacketQueue>();
demux_->setAudioQueue(audio_pkt_queue_);
demux_->setVideoQueue(video_pkt_queue_);
decodec_->setAudioQueue(audio_pkt_queue_);
decodec_->setVideoQueue(video_pkt_queue_);
decodec_->setAudioAvCodecInfo(audio_dec);
decodec_->setVideoAvCodecInfo(video_dec);
//需要在setAudioAvCodecInfo和setVideoAvCodecInfo之后调用
if(decodec_->Init()!=Ret_OK) {
    log_error("init decodec failed");
}
free(audio_dec);
free(video_dec);
audio_dec =nullptr;
video_dec =nullptr;
 }

VideoPlayer::~VideoPlayer()
{

}
void VideoPlayer::Play()
{
    demux_->StartReadPacket();
    decodec_->StartDecode();
}
//void VideoPlayer::Stop()
//{
//}
//void VideoPlayer::Pause()
//{
//}
//void VideoPlayer::SetPause(bool pause)
//{
//}
//bool VideoPlayer::GetPause()
//{
//}
//void VideoPlayer::SetPlay(bool play)
//{
//}
//bool VideoPlayer::GetPlay()
//{
//}
//void VideoPlayer::SetPosition(int position)
//{
//}
//int VideoPlayer::GetPosition()
//{
//}
//void VideoPlayer::SetVolume(int volume)
//{
//}
//int VideoPlayer::GetVolume()
//{
//}
