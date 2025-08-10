/**
 * 音声伝言板 (Raspberry Pi Pico 2 用)
 * 
 * @file FirFilter.cpp
 * @brief FIR フィルターの実装。
 * 
 * Copyright (C) 2025 Kentaro HARA.
 * https://kentaroh.jp/
 */

#include "FirFilter.h"

/// @brief コンストラクタ。
/// @param coefficients フィルター係数。
FirFilter::FirFilter(const std::vector<float>& coefficients)
    : _coeffs(coefficients), _buffer(coefficients.size(), 0.0f), _bufferIndex(0)
{
}

/// @brief 入力サンプルをフィルターに通します。
/// @param input 入力サンプル。
/// @return フィルター後の出力サンプル。
float FirFilter::Process(float input)
{
    // バッファに新しい入力を追加
    _buffer[_bufferIndex] = input;

    // フィルター出力を計算
    float output = 0.0f;
    for (size_t i = 0; i < _coeffs.size(); ++i)
    {
        output += _coeffs[i] * _buffer[(_bufferIndex + i) % _buffer.size()];
    }

    // バッファ インデックスを更新
    _bufferIndex = (_bufferIndex + 1) % _buffer.size();

    return output;
}

/// @brief フィルターの状態をリセットします。
void FirFilter::Reset()
{
    std::fill(_buffer.begin(), _buffer.end(), 0.0f);
    _bufferIndex = 0;
}