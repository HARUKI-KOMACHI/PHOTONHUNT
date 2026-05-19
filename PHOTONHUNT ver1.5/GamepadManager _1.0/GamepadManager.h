#pragma once

#define GAME_DEBUG

#ifndef GAME_DEBUG
#ifdef _DEBUG
#include "GamepadManager_DEDUG.h"
#endif // DEBUG
#endif // !GAME_DEBUG



#ifndef INCLUDE_IN
#define INCLUDE_IN

#include <iostream>  // 標準入出力ライブラリ。コンソールの入出力処理に使用
#include <Windows.h> // Windows API。ウィンドウ・スレッド・入力デバイス管理など、OSとのやり取りに必要な機能を提供
#include <Xinput.h>  // XInputライブラリ。Xboxコントローラーやその他の互換デバイスとのやり取りに使用
#include <string>    // C++標準ライブラリ。文字列操作のための std::string クラスを提供
#include <thread>    // C++11標準スレッドライブラリ。マルチスレッドの作成と管理に使用
#include <mutex>     // C++11標準ライブラリのミューテックスロック。スレッド同期およびリソース競合防止に使用
#include <locale>    // C++標準ライブラリ。ロケール設定と文字分類の処理に使用
#include <codecvt>   // C++標準ライブラリ。文字エンコード変換（ワイド文字とマルチバイト文字間の変換など）に使用
#include <sstream>   // C++標準ライブラリ。フォーマット付き入出力のための文字列ストリームを提供

// XInputライブラリをリンクし、Xbox互換コントローラーデバイスとのやり取りを可能にする
#pragma comment(lib, "Xinput.lib")

#include <dinput.h>  // DirectInputライブラリ。旧式コントローラーやジョイスティックなど、より広範な入力デバイスをサポート

// DirectInputに必要なライブラリをリンク
#pragma comment(lib, "dinput8.lib")  // DirectInput8ライブラリ。入力デバイス管理用のDirectInput APIをサポート
#pragma comment(lib, "dxguid.lib")   // DirectX GUIDライブラリ。DirectInputデバイス作成に必要なGUID定義を含む

#include <atomic>
#endif

// DirectInput デバイスインターフェースポインタの宣言。コントローラー入力制御に使用
extern LPDIRECTINPUTDEVICE8 pGameController;

// DirectInput を使用して振動を行う
void VibrateWithDirectInput(IDirectInputDevice8* pGameController, LONG magnitude);

// DirectInput コントローラーデバイス列挙のコールバック関数。使用可能なコントローラーデバイスを取得するために使用
BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);

// ゲームパッドパラメータクラスの定義。ボタンの状態やスティック・トリガーの値を格納する
#ifndef GAMEPAD_PAR
#define GAMEPAD_PAR

// スティックのX軸・Y軸の位置を格納するための2次元ベクトル構造体


#pragma once

// ===============================
// Gamepad Virtual Key (VK_PAD_*)
// ===============================

#define VK_PAD_NONE      0x0000

// メインボタン
#define VK_PAD_A         0x0001
#define VK_PAD_B         0x0002
#define VK_PAD_X         0x0003
#define VK_PAD_Y         0x0004

// ショルダーボタン
#define VK_PAD_L1        0x0010
#define VK_PAD_R1        0x0011
#define VK_PAD_L2        0x0012
#define VK_PAD_R2        0x0013

// ファンクションボタン
#define VK_PAD_SELECT    0x0020
#define VK_PAD_START     0x0021
#define VK_PAD_HOME      0x0022

// スティック押し込み
#define VK_PAD_LSTICK    0x0030
#define VK_PAD_RSTICK    0x0031

// 十字キー
#define VK_PAD_UP        0x0040
#define VK_PAD_DOWN      0x0041
#define VK_PAD_LEFT      0x0042
#define VK_PAD_RIGHT     0x0043


struct vector2
{
    int X;
    int Y;
};

class Gamepad_Parameters
{
public:
    // コントローラーボタンの状態変数。デフォルトは未押下（false）
    bool A = false;              // Aボタン（通常は確認やジャンプに使用）
    bool B = false;              // Bボタン（通常はキャンセルや攻撃に使用）
    bool X = false;              // Xボタン（通常はアクションや特殊スキルに使用）
    bool Y = false;              // Yボタン（通常は切り替えやインタラクションに使用）

    bool DPad_Up = false;        // 十字キー上（移動や選択に使用）
    bool DPad_Down = false;      // 十字キー下（移動や選択に使用）
    bool DPad_Left = false;      // 十字キー左（移動や選択に使用）
    bool DPad_Right = false;     // 十字キー右（移動や選択に使用）

    bool Left_Shoulder = false;  // 左ショルダーボタン（通常L1ボタン。補助操作に使用）
    bool Right_Shoulder = false; // 右ショルダーボタン（通常R1ボタン。補助操作に使用）

    bool Left_Trigger_Button = false;  // 左トリガーボタン（一部のコントローラーではトリガーもボタンとして扱う）
    bool Right_Trigger_Button = false; // 右トリガーボタン

    // 拡張ボタン：ボタン1〜4（その他のボタンへの拡張が可能）
    bool Button1 = false;  // 拡張ボタン1（コントローラーの実際のレイアウトに応じてマッピング）
    bool Button2 = false;  // 拡張ボタン2
    bool Button3 = false;  // 拡張ボタン3
    bool Button4 = false;  // 拡張ボタン4

    bool Button5 = false;  // 拡張ボタン5（サードパーティ製コントローラーのその他のファンクションキーに使用可能）
    bool Button6 = false;  // 拡張ボタン6
    bool Button7 = false;  // 拡張ボタン7
    bool Button8 = false;  // 拡張ボタン8


    // その他の拡張ファンクションキー（メニューボタン、オプションボタンなど）
    bool Menu_Button = false;     // メニューボタン
    bool Options_Button = false;  // オプションボタン
    bool Left_Middle_Button = false;  // コントローラー左側中央ボタン
    bool Right_Middle_Button = false; // コントローラー右側中央ボタン
    bool Central_Button = false;      // 中央ファンクションキー（Xboxのホームボタンやサードパーティのカスタムキーなどに対応）


    // スティックの位置。X軸・Y軸のデータを vector2 構造体で格納
    vector2 leftThumb = { 0, 0 };   // 左スティック
    vector2 rightThumb = { 0, 0 };  // 右スティック

    // トリガーの値（範囲：0〜255）
    int leftTrigger = 0;   // 左トリガー
    int rightTrigger = 0;  // 右トリガー

public:
    // コントローラーのボタン状態をリセットする関数。全ボタンの状態を false にする
    void Reset() {
        A = B = X = Y = DPad_Up = DPad_Down = DPad_Left = DPad_Right = false;
        Left_Shoulder = Right_Shoulder = false;
        Left_Trigger_Button = Right_Trigger_Button = false;

        // 全拡張ボタンをリセット
        Button1 = Button2 = Button3 = Button4 = false;
        Button5 = Button6 = Button7 = Button8 = false;

        // 中央ファンクションキーをリセット
        Menu_Button = Options_Button = Left_Middle_Button = Right_Middle_Button = Central_Button = false;
    }
};
#endif

// コントローラー検出管理クラス。コントローラーの入力検出・振動制御・状態処理などを担当
class GamepadDetectorManager
{
private:
    std::atomic<bool> isControllerThreadRunning = false;  // runControllerStateCheck() スレッドの重複起動を防ぐ

private:
    std::string pressedButtons;  // 押されているボタン名を格納
    WORD buttonStates;        // ボタン状態のビット表現を格納
    std::mutex stateMutex; // ミューテックスロック。コントローラー状態へのスレッドセーフなアクセスを保証

    // コントローラーの振動を設定する。左右それぞれの振動モーターの強度は 0〜65535
    // コントローラー状態の取得関数。入力を処理する
    void GetControllerState();
    // 初期化関数。DirectInputの初期化などを含む可能性がある
    void initialize();
    // 独立スレッドでコントローラー状態を取得する
    void runControllerStateCheck();

    void gamepadAvailable();

private:

    int leftVibrateController = 0;  // 左モーターの振動強度
    int rightVibrateController = 0; // 右モーターの振動強度

#ifndef GAME_DEBUG
#ifdef _DEBUG
    createWindow Gamepad_Parameters_win;
#endif // DEBUG
#endif

public:
    // コントローラーの振動制御に使用
    bool vibrateController = false;  // コントローラーが振動しているかどうかの状態
    std::string Detected_Device = ""; // 検出されたデバイス名を格納
    const int stickSensitivity = 2000;  // スティック感度のスケーリング係数
private:
    Gamepad_Parameters gamepad_Parameters; // コントローラーパラメータのインスタンス
    Gamepad_Parameters old_gamepad_Parameters; // コントローラーパラメータのインスタンス（前フレーム）

public:
    void VibrateController(WORD leftMotorSpeed, WORD rightMotorSpeed);
    // ウィンドウを起動し、独立スレッドで出力を表示する
    void run();

    bool IsPressed(const std::string t);
    bool JustPressed(const std::string t);
    bool JustReleased(const std::string t);
    bool IsAnyInput();

    vector2& GetleftThumb(void);
    vector2& GetrightThumb(void);

    int& GetleftTrigger(void);
    int& GetrightTrigger(void);


    float GetleftThumbAngle(void);
    float GetleftThumbAngleDeg(void);
    float GetrightThumbAngle(void);
    float GetrightThumbAngleDeg(void);


};


