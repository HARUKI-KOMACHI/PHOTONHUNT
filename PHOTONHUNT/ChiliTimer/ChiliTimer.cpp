

// ChiliTimer.cpp
// =========================
// このファイルは ChiliTimer クラスのメソッドを実装します
// =========================

#include "ChiliTimer.h"

using namespace std::chrono;

// コンストラクタ：ChiliTimerインスタンスを作成する際に、現在の時刻を記録する
ChiliTimer::ChiliTimer() noexcept
{
    last = steady_clock::now(); // 记录当前时间点
}

// Mark(): 前回の Mark() 呼び出しからの経過時間を計算し、タイムスタンプを更新する
float ChiliTimer::Mark() noexcept
{
    const auto old = last; // 古い時点のバックアップ
    last = steady_clock::now(); // 新しい時点を記録する
    const duration<float> frameTime = last - old; // 時間差の計算（単位：秒）
    return frameTime.count(); // 戻り時間差（浮動小数点数、単位：秒）
}

// Peek(): 現在の時刻と前回の Mark() 呼び出しの間の時間を計算する（タイムスタンプをリセットしない）
float ChiliTimer::Peek() const noexcept
{
    return duration<float>(steady_clock::now() - last).count(); // 戻り時間差
}


float ChiliTimer::Now() const noexcept
{
    return duration<float>(
        steady_clock::now().time_since_epoch()
    ).count();
}

