/**
 * 音声伝言板 (Raspberry Pi Pico 2 用)
 * 
 * @file VoiceMessage.h
 * @brief 音声メッセージの録音と再生を処理するクラス。
 * 
 * Copyright (C) 2025 Kentaro HARA.
 * https://kentaroh.jp/
 */

#ifndef VOICEMESSAGE_H
#define VOICEMESSAGE_H

#include <string>
#include <vector>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "FirFilter.h"

/// @brief 音声メッセージの録音と再生を処理するクラス。
class VoiceMessage
{
public:
    VoiceMessage();
    ~VoiceMessage();
    // 音声メッセージの録音を開始
    bool StartRecording();
    // 音声メッセージの再生を開始
    bool StartPlayback();
    // 音声メッセージの録音/再生を停止
    void Stop();
    /// @brief 処理中かどうかを取得します。
    /// @return 処理中であれば true、そうでなければ false を返します。
    bool IsBusy() const { return _isBusy; }

private:
    enum Config
    {
        DECIM_RATE = 4,                                       // ダウン サンプリング率
        INTERP_RATE = DECIM_RATE,                             // オーバー サンプリング率
        BASE_SAMPLE_RATE = 8000,                              // ベース サンプリング レート (8 kHz)
        SAMPLE_RATE = BASE_SAMPLE_RATE * INTERP_RATE,         // サンプリング レート
        TIMER_PERIOD_US = 1000000 / SAMPLE_RATE,              // タイマー周期 (マイクロ秒)
        TIME_LENGTH = 30,                                     // 音声メッセージの最大長 (秒)
        MAX_SAMPLES = SAMPLE_RATE * TIME_LENGTH / DECIM_RATE, // 最大サンプル数
        BUFFER_SIZE = MAX_SAMPLES,                            // 音声データのバッファ サイズ (バイト数)
        DAC_BITS = 10,                                        // DAC のビット数
        PWM_WRAP_VALUE = (1 << DAC_BITS) - 1,                 // PWM のラップ値
        // GPIO 設定
        ADC_GPIO_NUM = 26,                                    // ADC 入力に使用する GPIO 番号
        PWM_GPIO_NUM = 0,                                     // PWM 出力に使用する GPIO 番号
        MPX_GPIO_NUM = 1,                                     // マルチプレクサ出力に使用する GPIO 番号
    };
    repeating_timer_t _timer;         // タイマーのインスタンス
    bool _timerStarted;               // タイマーが開始されているかどうか
    uint32_t _sampleCount;            // 現在のサンプル数
    size_t _sampleIndex;              // サンプルのインデックス
    bool _isRecordingMode;            // 録音モードかどうか
    bool _isBusy;                     // 処理中かどうか
    std::vector<int8_t> _audioBuffer; // 音声データのバッファ
    FirFilter _lowpassFilter;         // ローパス フィルター
    uint _pwmSliceNum;                // PWM スライス番号
    void Initialize();
    bool StartTimer();
    void StopTimer();
    bool StartProcessing();
    static bool TimerCallback(repeating_timer_t *rt);
    void OnTimer();
    /// @brief 飽和演算 (クリッピング) を行います。
    /// @param value 対象となる値。
    /// @param min 下限値。
    /// @param max 上限値。
    /// @return 演算後の値。
    int16_t Saturate(int16_t value, int16_t min, int16_t max)
    {
        return (value < min) ? min : (value > max) ? max : value;
    }
};

#endif // VOICEMESSAGE_H
// End of VoiceMessage.h