#include "../main.hpp"
#include "AXSetVoiceDeviceMix.hpp"

bool AXSetVoiceDeviceMix::isStarted = false; // 開始フラグ
bool AXSetVoiceDeviceMix::isSwap = false; // 画面スワップのフラグ

DECL_FUNCTION(AXResult, AXSetVoiceDeviceMix, AXVoice *voice, AXDeviceType type, uint32_t id, AXVoiceDeviceMixData *mixData) // ラッパー関数の実装
{
    AXSetVoiceDeviceMix::SwapMixDevice(&type); // クラス内のbool値によって音声スワップを適用
    if(!AXSetVoiceDeviceMix::GetIsStarted()) // フラグが立っていない場合、Startを呼び出す
    {
        AXSetVoiceDeviceMix::Start(voice, type, id, mixData); // 最初の一回だけ呼ばれる
        AXSetVoiceDeviceMix::SetIsStarted(true);             // フラグを立てる
    }
    AXSetVoiceDeviceMix::Update(voice, type, id, mixData); // 繰り返し呼ばれる
    return real_AXSetVoiceDeviceMix(voice, type, id, mixData); // 本来呼ばれる関数を呼び出す
}

WUPS_MUST_REPLACE_FOR_PROCESS(AXSetVoiceDeviceMix, WUPS_LOADER_LIBRARY_SNDCORE2, AXSetVoiceDeviceMix, WUPS_FP_TARGET_PROCESS_ALL); // ラッパー関数と本来の関数を置き換える

void AXSetVoiceDeviceMix::Start(AXVoice *voice, AXDeviceType type, uint32_t id, AXVoiceDeviceMixData *mixData)
{

}

void AXSetVoiceDeviceMix::Update(AXVoice *voice, AXDeviceType type, uint32_t id, AXVoiceDeviceMixData *mixData)
{
    NotificationModule_AddInfoNotification("AudioMixed!!");
}