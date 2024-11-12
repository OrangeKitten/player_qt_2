#ifndef __TYPE_H__
#define __TYPE_H__

enum Ret {
    Ret_OK = 0,
    Ret_ERROR = -1,
    Ret_ERROR_EXIT = -2,
};
enum class PlayerState {
    PlayerState_Stop = 0,
    PlayerState_Ready,
    PlayerState_Play,
    PlayerState_Pause,
    PlayerState_Seek,
};
typedef struct AUDIO_INFO {
    int freq;
    int channels;
    int format;
    int samples;
    int silence;
    void *userdata;
}Auido_Info;
#endif