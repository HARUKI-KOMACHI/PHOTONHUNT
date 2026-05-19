#pragma once
// ChiliTimer.h
// =========================
// このクラスは時間間隔を測定するために使用され、
// フレーム時間計算（ゲームループなど）に適しています。
// =========================

#include <chrono> // Chronoを導入して高精度な計時を実現する

class ChiliTimer
{
public:
    // コンストラクタ：初期化時に現在の時刻を記録する
    ChiliTimer() noexcept;

    // タイムスタンプを記録し、前回の Mark() 呼び出しからの経過時間（秒）を返す
    float Mark() noexcept;

    // 現在の時刻と前回の Mark() 呼び出しとの間の時間（タイムスタンプをリセットしない）
    float Peek() const noexcept;
    //戻り時間
    float Now() const noexcept;

private:
    std::chrono::steady_clock::time_point last; // 前回の Mark() 呼び出しの時刻を記録する
};