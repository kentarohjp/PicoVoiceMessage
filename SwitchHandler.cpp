/**
 * 音声伝言板 (Raspberry Pi Pico 2 用)
 * 
 * @file SwitchHandler.cpp
 * @brief スイッチの入力を処理するクラス。
 * 
 * Copyright (C) 2025 Kentaro HARA.
 * https://kentaroh.jp/
 */

#include "SwitchHandler.h"
#include <pico/stdlib.h>
#include <hardware/gpio.h>

/// @brief コンストラクタ。
SwitchHandler::SwitchHandler(VoiceMessage &voiceMessage)
    : _voiceMessage(voiceMessage)
{
    _recPressCount = 0; // 録音スイッチの押下時間カウントを初期化
    // GPIO の初期化を行います。
    Initialize();
}

/// @brief GPIO の初期化を行います。
void SwitchHandler::Initialize()
{
    // GPIO の初期化を行います。
    // 録音用スイッチの初期化
    gpio_init(Config::SWITCH_RECORD);
    gpio_set_dir(Config::SWITCH_RECORD, GPIO_IN);
    gpio_pull_up(Config::SWITCH_RECORD); // プルアップ抵抗を有効にする

    // 再生用スイッチの初期化
    gpio_init(Config::SWITCH_PLAYBACK);
    gpio_set_dir(Config::SWITCH_PLAYBACK, GPIO_IN);
    gpio_pull_up(Config::SWITCH_PLAYBACK); // プルアップ抵抗を有効にする

    // 停止用スイッチの初期化
    gpio_init(Config::SWITCH_STOP);
    gpio_set_dir(Config::SWITCH_STOP, GPIO_IN);
    gpio_pull_up(Config::SWITCH_STOP); // プルアップ抵抗を有効にする
}

/// @brief スイッチの入力を処理します。
void SwitchHandler::HandleSwitch()
{
    // 録音または再生中の場合
    if (_voiceMessage.IsBusy())
    {
        // 停止用スイッチが押された場合
        if (!gpio_get(Config::SWITCH_STOP))
        {
            // 音声メッセージの録音/再生を停止
            _voiceMessage.Stop();
        }
    }
    else
    {
        // 録音用スイッチが押された場合
        if (!gpio_get(Config::SWITCH_RECORD))
        {
            // 録音スイッチの押下時間をカウント
            _recPressCount++;

            // 一定時間以上押されている場合は録音を開始
            if (_recPressCount > REC_PRESS_MAX) // 100 ms * 10 回 = 1000 ms
            {
                // 音声メッセージの録音を開始
                _voiceMessage.StartRecording();
            }
        }
        // 再生用スイッチが押された場合
        else if (!gpio_get(Config::SWITCH_PLAYBACK))
        {
            // 音声メッセージの再生を開始
            _voiceMessage.StartPlayback();
        }
    }

    // 録音用スイッチが離された場合、押下時間カウントをリセット
    if (gpio_get(Config::SWITCH_RECORD))
    {
        _recPressCount = 0;
    }
}
