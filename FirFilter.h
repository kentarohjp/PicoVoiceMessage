/**
 * 音声伝言板 (Raspberry Pi Pico 2 用)
 * 
 * @file FirFilter.h
 * @brief FIR フィルターのヘッダー ファイル。
 * 
 * Copyright (C) 2025 Kentaro HARA.
 * https://kentaroh.jp/
 */

#ifndef FIR_FILTER_H
#define FIR_FILTER_H
#include <vector>

/// @brief FIR フィルター クラス。
/// @details このクラスは FIR フィルターを実装し、与えられた係数を使用して入力サンプルを処理します。
class FirFilter
{
public:
    FirFilter(const std::vector<float>& coefficients);
    float Process(float input);
    void Reset();

private:
    std::vector<float> _coeffs; // フィルター係数
    std::vector<float> _buffer; // 入力サンプルのバッファ
    size_t _bufferIndex;        // バッファの現在のインデックス
};

#endif // FIR_FILTER_H