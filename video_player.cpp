#include "video_player.h"
#include "log.h"
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
audio_pkt_queue_ = std::make_shared<PacketQueue>();
video_pkt_queue_ = std::make_shared<PacketQueue>();
demux_->setAudioQueue(audio_pkt_queue_);
demux_->setVideoQueue(video_pkt_queue_);
decodec_->setAudioQueue(audio_pkt_queue_);
decodec_->setVideoQueue(video_pkt_queue_);
 }

VideoPlayer::~VideoPlayer()
{

}
void VideoPlayer::Play()
{
    demux_->StartReadPacket();
    decodec_->StartDecode();
}
void VideoPlayer::Stop()
{
}
void VideoPlayer::Pause()
{
}
void VideoPlayer::SetPause(bool pause)
{
}
bool VideoPlayer::GetPause()
{
}
void VideoPlayer::SetPlay(bool play)
{
}
bool VideoPlayer::GetPlay()
{
}
void VideoPlayer::SetPosition(int position)
{
}
int VideoPlayer::GetPosition()
{
}
void VideoPlayer::SetVolume(int volume)
{
}
int VideoPlayer::GetVolume()
{
}