#include "video_player.h"
extern "C" {
#include "log.h"
}
#define MaxPack 8
VideoPlayer::VideoPlayer (char *url)
{
     position_ =-1;
     volume_ =-1;
     url_ = url;
     pause_state_ =false;
     last_pause_state_ =true;
}

Ret  VideoPlayer::init(){
demux_ = std::make_unique<Demux>(url_);
decodec_ = std::make_unique<Decodec>();

AVCodecParameters *audio_dec = (AVCodecParameters*)malloc(sizeof(AVCodecParameters));
AVCodecParameters *video_dec = (AVCodecParameters*)malloc(sizeof(AVCodecParameters));
demux_->getAudioAvCodecInfo(audio_dec);
demux_->getVideoAvCodecInfo(video_dec);
AVRational video_time_base = demux_->getVideoTimeBase();
AVRational audio_time_base = demux_->getAudioTimeBase();
AVRational video_frame_rate = demux_->getVideoFrameRate();
//创建AvpacketQueue
audio_pkt_queue_ = std::make_shared<PacketQueue>(MaxPack,"audiopkt");
video_pkt_queue_ = std::make_shared<PacketQueue>(MaxPack,"videopkt");
demux_->setAudioQueue(audio_pkt_queue_);
demux_->setVideoQueue(video_pkt_queue_);
decodec_->setAudioQueue(audio_pkt_queue_);
decodec_->setVideoQueue(video_pkt_queue_);
decodec_->setAudioAvCodecInfo(audio_dec);
decodec_->setVideoAvCodecInfo(video_dec);


decodec_->set_audio_timebase(audio_time_base);
decodec_->set_video_timebase(video_time_base);
decodec_->set_video_frame_rate(video_frame_rate);
//需要在setAudioAvCodecInfo和setVideoAvCodecInfo之后调用
if(decodec_->Init()!=Ret_OK) {
    log_error("init decodec failed");
}
free(audio_dec);
free(video_dec);
audio_dec =nullptr;
video_dec =nullptr;
player_state_ = PlayerState::PlayerState_Ready;
decodec_->set_player_state(player_state_);
return Ret_OK;
 }

VideoPlayer::~VideoPlayer()
{

}
void VideoPlayer::Play()
{
    player_state_ = PlayerState::PlayerState_Play;
    decodec_->set_player_state(player_state_);
    demux_->StartReadPacket();
    decodec_->StartDecode();
  
        // 打开 YUV 文件
    //decodec_->VideoRendor();
    

}


void VideoPlayer::Stop()
{
    player_state_ = PlayerState::PlayerState_Stop;
    decodec_->set_player_state(player_state_);
}
void VideoPlayer::Pause()
{
    if(last_pause_state_ !=pause_state_){
        last_pause_state_ = pause_state_;
        pause_state_ =!pause_state_;
    } 
    
    player_state_ = pause_state_?PlayerState::PlayerState_Pause:PlayerState::PlayerState_Play;
    log_debug("pause state = %d\n",player_state_);
   decodec_->set_player_state(player_state_);
}
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

   void VideoPlayer::set_volume(int volume) {
decodec_->set_volume(volume);
   }
   int VideoPlayer::get_volume() {
 return decodec_->get_volume();
   }
   Ret VideoPlayer::set_mute(bool mute) {
    decodec_->set_mute(mute);
   }

PlayerState VideoPlayer::getState() {
    std::lock_guard<std::mutex> lock(mtx_);
    return player_state_;
}
