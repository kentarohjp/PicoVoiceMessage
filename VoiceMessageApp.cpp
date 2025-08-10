/**
 * 音声伝言板 (Raspberry Pi Pico 2 用)
 * 
 * @file VoiceMessageApp.cpp
 * @brief 音声伝言板アプリケーションのエントリー ポイント。
 * 
 * Copyright (C) 2025 Kentaro HARA.
 * https://kentaroh.jp/
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "VoiceMessage.h"
#include "SwitchHandler.h"

/// @brief メイン関数。
int main()
{
    stdio_init_all();

    VoiceMessage voiceMessage;
    SwitchHandler switchHandler(voiceMessage);

    while (true)
    {
        switchHandler.HandleSwitch(); // スイッチの入力を処理
        sleep_ms(100);                // 100 ms 待機
    }
}
