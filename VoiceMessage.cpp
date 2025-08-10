/**
 * 音声伝言板 (Raspberry Pi Pico 2 用)
 * 
 * @file VoiceMessage.cpp
 * @brief 音声メッセージの録音と再生を処理するクラス。
 * 
 * Copyright (C) 2025 Kentaro HARA.
 * https://kentaroh.jp/
 */

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "VoiceMessage.h"
#include "FilterCoeffs.h" // フィルター係数 (lowpassCoeffs)

/// @brief コンストラクタ。
VoiceMessage::VoiceMessage()
: _lowpassFilter(lowpassCoeffs) // ローパス フィルターの初期化
{
    // コンストラクタの初期化処理
    _timerStarted = false;             // タイマー開始状態を初期化
    _sampleCount = 0;                  // 現在のサンプル数を初期化
    _sampleIndex = 0;                  // サンプルのインデックスを初期化
    _isRecordingMode = false;          // 録音モードを初期化
    _isBusy = false;                   // 処理中フラグを初期化
    _pwmSliceNum = 0;                  // PWM スライス番号を初期化
    _audioBuffer.reserve(BUFFER_SIZE); // 音声データのバッファを予約
    _lowpassFilter.Reset();            // フィルターの状態をリセット

    // デバイスの初期化
    Initialize();
}

/// @brief デストラクタ。
VoiceMessage::~VoiceMessage()
{
}

/// @brief 音声メッセージの録音を開始します。
/// @return 開始できた場合は true、失敗した場合は false を返します。
/// @details 既に処理中の場合は録音を開始しません。
bool VoiceMessage::StartRecording()
{
    if (_isBusy) // 既に処理中の場合は録音を開始しない
    {
        return false; // 処理中の場合は false を返す
    }

    _isRecordingMode = true; // 録音モードに設定
    _audioBuffer.clear();    // 音声データのバッファをクリア

    return StartProcessing(); // タイマーを開始して録音を開始
}

/// @brief 音声メッセージの再生を開始します。
/// @return 開始できた場合は true、失敗した場合は false を返します。
/// @details 既に処理中の場合は再生を開始しません。
bool VoiceMessage::StartPlayback()
{
    if (_isBusy) // 既に処理中の場合は再生を開始しない
    {
        return false; // 処理中の場合は false を返す
    }

    if (_audioBuffer.empty()) // バッファが空の場合は再生できない
    {
        return false; // バッファが空の場合は false を返す
    }

    _isRecordingMode = false;            // 再生モードに設定
    _sampleIndex = 0;                    // サンプルのインデックスをリセット
    pwm_set_enabled(_pwmSliceNum, true); // PWM を有効化

    return StartProcessing(); // タイマーを開始して再生を開始
}

/// @brief 音声メッセージの録音/再生を停止します。
void VoiceMessage::Stop()
{
    // 録音/再生を停止する処理
    StopTimer();                          // タイマーを停止
    _isBusy = false;                      // 処理中フラグをリセット
    _sampleCount = 0;                     // 現在のサンプル数をリセット
    _lowpassFilter.Reset();               // フィルターの状態をリセット
    pwm_set_enabled(_pwmSliceNum, false); // PWM を無効化
    gpio_put(PICO_DEFAULT_LED_PIN, 0);    // LED を消灯
}

/// @brief 周辺機能の初期化を行います。
/// @details ADC、PWM、GPIO の初期化を行います。
void VoiceMessage::Initialize()
{
    // ADC の初期化
    adc_init();
    adc_gpio_init(ADC_GPIO_NUM); // ADC_GPIO_NUM を ADC0 に設定
    adc_select_input(0);         // ADC0 を選択

    // PWM の初期化
    gpio_set_function(PWM_GPIO_NUM, GPIO_FUNC_PWM);     // GPIO を PWM 出力に設定
    _pwmSliceNum = pwm_gpio_to_slice_num(PWM_GPIO_NUM); // GPIO のスライス番号を取得
    pwm_set_wrap(_pwmSliceNum, PWM_WRAP_VALUE);         // PWM のラップ値を設定 (10 ビット相当)
    pwm_set_clkdiv(_pwmSliceNum, 1.0f);                 // クロック ディバイダを設定 (1.0f = 1/1)

    // GPIO の初期化
    gpio_init(PICO_DEFAULT_LED_PIN);              // LED ピンの初期化
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT); // LED ピンを出力に設定
    gpio_init(MPX_GPIO_NUM);                      // マルチプレクサ出力ピンの初期化
    gpio_set_dir(MPX_GPIO_NUM, GPIO_OUT);         // マルチプレクサ出力ピンを出力に設定
}

/// @brief タイマーを開始します。
/// @return タイマーを開始できた場合は true、失敗した場合は false を返します。
/// @details タイマーが既に開始されている場合は何もしません。
bool VoiceMessage::StartTimer()
{
    if (_timerStarted)
    {
        // タイマーが既に開始されている場合は、何もしません
        return true;
    }

    // タイマーの初期化
    _timerStarted = add_repeating_timer_us(-TIMER_PERIOD_US, TimerCallback, this, &_timer);
    return _timerStarted;
}

/// @brief タイマーを停止します。
void VoiceMessage::StopTimer()
{
    if (_timerStarted)
    {
        // タイマーを停止します
        cancel_repeating_timer(&_timer);
        _timerStarted = false;
    }
}

/// @brief 録音または再生の処理を開始します。
/// @return 処理を開始できた場合は true、失敗した場合は false を返します。
/// @details タイマーを開始し、録音または再生の処理を行います。
bool VoiceMessage::StartProcessing()
{
    gpio_put(MPX_GPIO_NUM, _isRecordingMode); // マルチプレクサの出力を設定
    sleep_ms(1);                              // マルチプレクサの設定が反映されるまで待機

    if (!StartTimer()) // タイマーを開始
    {
        return false; // タイマー開始に失敗した場合は false を返す
    }

    _isBusy = true;                    // 処理中フラグをセット
    gpio_put(PICO_DEFAULT_LED_PIN, 1); // LED を点灯
    return true;
}

/// @brief タイマー コールバック関数。
bool VoiceMessage::TimerCallback(repeating_timer_t *rt)
{
    // タイマー コールバックの処理
    VoiceMessage *instance = static_cast<VoiceMessage *>(rt->user_data);
    instance->OnTimer();
    return true; // true を返すと、タイマーは再度呼び出されます
}

/// @brief タイマー イベントの処理を行います。
/// @details 録音または再生の処理を行います。
void VoiceMessage::OnTimer()
{
    // タイマー イベントの処理
    if (!_isBusy)
    {
        // 処理中でない場合は何もしません
        return;
    }

    if (_isRecordingMode)
    {
        // 録音モードの処理
        uint16_t adcValue = adc_read(); // ADC からの値を読み取る
        // ADC の値は 0 から 4095 の範囲なので、-2048 から +2047 の範囲に変換後、LPF で 4 kHz 以上をカット
        float filteredValue = _lowpassFilter.Process(adcValue - 2048.0f); // フィルターを通す
        // データを間引いてダウン サンプリング (デシメーション) を行う
        if (_sampleCount % DECIM_RATE == 0) // DECIM_RATE サンプルごとに処理
        {
            // 12 ビットから 8 ビット (-128 から +127) にスケーリング
            int16_t scaledValue = Saturate(static_cast<int16_t>(filteredValue / 16.0f), -128, 127); // スケーリング
            _audioBuffer.push_back(static_cast<int8_t>(scaledValue));                               // スケーリング後の値をバッファに追加
            if (_audioBuffer.size() >= _audioBuffer.capacity())                                     // バッファ サイズに達したら録音を停止
            {
                Stop(); // 録音を停止
            }
        }
    }
    else
    {
        // 再生モードの処理
        // サンプル間に (INTERP_RATE - 1) 個の 0 データをフィルすることにより、オーバー サンプリングを行う
        int8_t data; // 再生するデータ
        if (_sampleCount % INTERP_RATE == 0)
        {
            data = _audioBuffer[_sampleIndex]; // バッファからサンプルを取得
            _sampleIndex++;                    // サンプルのインデックスをインクリメント
        }
        else
        {
            data = 0; // 0 データを挿入
        }
        // フィルターで 4 kHz 以上の信号をカットし、さらに振幅を INTERP_RATE 倍します
        float filteredValue = _lowpassFilter.Process(data) * INTERP_RATE;
        // フィルター後の値を 0 から 1023 の範囲にスケーリング (8 ビット → 10 ビット)
        int16_t pwmValue = Saturate(static_cast<int16_t>((filteredValue + 128.0f) * 4.0f), 0, PWM_WRAP_VALUE); // 10 ビットにスケーリング
        // PWM のデューティ サイクルを設定 
        pwm_set_gpio_level(PWM_GPIO_NUM, static_cast<uint16_t>(pwmValue)); // PWM 出力に値を設定
        if (_sampleIndex >= _audioBuffer.size()) // インデックスがバッファのサイズに達した場合
        {
            Stop(); // 再生を停止
        }
    }
    
    _sampleCount++; // サンプル数をインクリメント
}
