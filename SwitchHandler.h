/**
 * 音声伝言板 (Raspberry Pi Pico 2 用)
 * 
 * @file SwitchHandler.h
 * @brief スイッチの入力を処理するクラス。
 * 
 * Copyright (C) 2025 Kentaro HARA.
 * https://kentaroh.jp/
 */

#ifndef SWITCH_HANDLER_H
#define SWITCH_HANDLER_H

#include "VoiceMessage.h"

/// @brief スイッチの入力を処理するクラス。
/// @details このクラスは、録音、再生、および停止のスイッチ入力を処理し、VoiceMessage クラスと連携します。
class SwitchHandler
{
public:
    SwitchHandler(VoiceMessage& voiceMessage);
    void HandleSwitch();

private:
    enum Config
    {
        SWITCH_RECORD = 2,   // 録音用スイッチの GPIO 番号
        SWITCH_PLAYBACK = 3, // 再生用スイッチの GPIO 番号
        SWITCH_STOP = 4,     // 停止用スイッチの GPIO 番号
        REC_PRESS_MAX = 10,  // 録音スイッチの押下時間カウントの最大値
    };

    VoiceMessage &_voiceMessage; // VoiceMessage インスタンスへの参照
    uint32_t _recPressCount;     // 録音スイッチの押下時間カウント
    void Initialize();
};

#endif // SWITCH_HANDLER_H