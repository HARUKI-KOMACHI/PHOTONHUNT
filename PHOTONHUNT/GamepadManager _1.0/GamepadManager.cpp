#include "GamepadManager.h"


LPDIRECTINPUT8 pDirectInput = NULL;
LPDIRECTINPUTDEVICE8 pGameController = NULL;
std::string name;

// TCHAR を std::string に変換する
std::string TCHARToString(const TCHAR* tcharStr) {
    // プロジェクトが Unicode モードの場合
#ifdef UNICODE
    int len = WideCharToMultiByte(CP_ACP, 0, tcharStr, -1, NULL, 0, NULL, NULL);
    std::string str(len, '\0');
    WideCharToMultiByte(CP_ACP, 0, tcharStr, -1, &str[0], len, NULL, NULL);
    return str;
#else
    // プロジェクトがマルチバイトモードの場合、そのまま文字列を返す
    return std::string(tcharStr);
#endif
}

// 感度調整関数。スティック入力値を微調整し、値が非常に小さい場合は 0 にする
int new_thuml(int old) {
    if (old < 2 && old > -2) {
        old = 0;
    }
    return old;
}

// どのボタンが押されているかを検出する
std::string GetPressedButtons(WORD buttons, Gamepad_Parameters& gamepad_Parameters) {
    std::string result = "";

    gamepad_Parameters.Reset();  // コントローラー状態をリセットし、毎回最新の状態を検出できるようにする

    // 一般的なボタンを検出
    if (buttons & XINPUT_GAMEPAD_A) {
        gamepad_Parameters.A = true;
        result += "A ";
    }
    if (buttons & XINPUT_GAMEPAD_B) {
        gamepad_Parameters.B = true;
        result += "B ";
    }
    if (buttons & XINPUT_GAMEPAD_X) {
        gamepad_Parameters.X = true;
        result += "X ";
    }
    if (buttons & XINPUT_GAMEPAD_Y) {
        gamepad_Parameters.Y = true;
        result += "Y ";
    }

    // 十字キーを検出
    if (buttons & XINPUT_GAMEPAD_DPAD_UP) {
        gamepad_Parameters.DPad_Up = true;
        result += "DPad Up ";
    }
    if (buttons & XINPUT_GAMEPAD_DPAD_DOWN) {
        gamepad_Parameters.DPad_Down = true;
        result += "DPad Down ";
    }
    if (buttons & XINPUT_GAMEPAD_DPAD_LEFT) {
        gamepad_Parameters.DPad_Left = true;
        result += "DPad Left ";
    }
    if (buttons & XINPUT_GAMEPAD_DPAD_RIGHT) {
        gamepad_Parameters.DPad_Right = true;
        result += "DPad Right ";
    }

    // ショルダーボタンを検出
    if (buttons & XINPUT_GAMEPAD_LEFT_SHOULDER) {
        gamepad_Parameters.Left_Shoulder = true;
        result += "Left Shoulder ";
    }
    if (buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
        gamepad_Parameters.Right_Shoulder = true;
        result += "Right Shoulder ";
    }

    // MenuボタンやOptionsボタンなどの拡張ボタンを検出
    if (buttons & XINPUT_GAMEPAD_BACK) {
        gamepad_Parameters.Menu_Button = true;
        result += "Menu Button ";
    }
    if (buttons & XINPUT_GAMEPAD_START) {
        gamepad_Parameters.Options_Button = true;
        result += "Options Button ";
    }


    // ボタンが何も押されていない場合
    if (result == "") {
        result = "No buttons pressed";
    }

    return result;
}


// コントローラーの状態情報を取得し、ボタンとスティックの状態を表示するとともに振動ロジックを処理する
void GamepadDetectorManager::GetControllerState() {
    if (Detected_Device.find("Xbox") != std::string::npos) {
        // XInput を使用して Xbox コントローラーの入力を読み取る
        XINPUT_STATE state;
        ZeroMemory(&state, sizeof(XINPUT_STATE));
        //VibrateController(0, 0);

        // コントローラーの状態を取得
        DWORD dwResult = XInputGetState(0, &state);
        if (dwResult == ERROR_SUCCESS) {
            // スティックとトリガーの値を取得・処理する
            // ロックを取得してコントローラー状態を更新
            std::lock_guard<std::mutex> lock(stateMutex);

            gamepad_Parameters.leftThumb.X = new_thuml(state.Gamepad.sThumbLX / stickSensitivity); // 左スティックX値
            gamepad_Parameters.leftThumb.Y = new_thuml(state.Gamepad.sThumbLY / stickSensitivity); // 左スティックY値
            gamepad_Parameters.rightThumb.X = new_thuml(state.Gamepad.sThumbRX / stickSensitivity);// 右スティックX値
            gamepad_Parameters.rightThumb.Y = new_thuml(state.Gamepad.sThumbRY / stickSensitivity);// 右スティックY値

            gamepad_Parameters.leftTrigger = (int)state.Gamepad.bLeftTrigger;   // 左トリガーの値
            gamepad_Parameters.rightTrigger = (int)state.Gamepad.bRightTrigger; // 右トリガーの値



            buttonStates = state.Gamepad.wButtons;
            pressedButtons = GetPressedButtons(buttonStates, gamepad_Parameters);


            // トリガーが押された場合、振動パラメータを設定する
            //if (true) {
            //    // 設定された振動パラメータに基づいて振動関数を呼び出す
            //    VibrateController(state.Gamepad.bLeftTrigger, state.Gamepad.bRightTrigger);
            //}
#ifndef GAME_DEBUG
#ifdef _DEBUG
            gamepad_Parameters_win.text_out(pressedButtons, buttonStates, gamepad_Parameters);
#endif // DEBUG
#endif
        }
    }
    else {
        // DirectInput を使用してサードパーティ製コントローラーの入力を読み取る
        if (pGameController) {
            DIJOYSTATE2 js; // DirectInput のコントローラー状態構造体

            if (FAILED(pGameController->Poll())) {
                // Poll が失敗した場合は何もせず、直接コントローラーの状態を取得する
                // コントローラーが Poll モードをサポートしていない可能性がある
                if (FAILED(pGameController->Acquire())) {
                    MessageBox(NULL, L"Failed to acquire the game controller after Poll failure.", L"question", MB_OK | MB_ICONERROR);
                    return;
                }
            }

            if (SUCCEEDED(pGameController->GetDeviceState(sizeof(DIJOYSTATE2), &js))) {
                // ロックを取得してコントローラー状態を更新
                std::lock_guard<std::mutex> lock(stateMutex);

                // 左スティックを処理
                gamepad_Parameters.leftThumb.X = new_thuml(js.lX / stickSensitivity); // 左スティックX値
                gamepad_Parameters.leftThumb.Y = new_thuml(js.lY / stickSensitivity); // 左スティックY値

                // 右スティックを処理
                gamepad_Parameters.rightThumb.X = new_thuml(js.lRx / stickSensitivity); // 右スティックX値
                gamepad_Parameters.rightThumb.Y = new_thuml(js.lRy / stickSensitivity); // 右スティックY値

                // トリガーを処理
                gamepad_Parameters.leftTrigger = js.lZ;   // DirectInput の Z軸は通常トリガーを表す
                gamepad_Parameters.rightTrigger = js.rglSlider[0]; // rglSlider[0] はもう一方のトリガーを表すことができる

                // ボタンを処理
                for (int i = 0; i < 128; ++i) {
                    if (js.rgbButtons[i] & 0x80) {
                        switch (i) {
                        case 0: gamepad_Parameters.A = true; break;
                        case 1: gamepad_Parameters.B = true; break;
                        case 2: gamepad_Parameters.X = true; break;
                        case 3: gamepad_Parameters.Y = true; break;
                        case 4: gamepad_Parameters.Left_Shoulder = true; break;
                        case 5: gamepad_Parameters.Right_Shoulder = true; break;
                        case 6: gamepad_Parameters.Left_Trigger_Button = true; break;
                        case 7: gamepad_Parameters.Right_Trigger_Button = true; break;
                        case 8: gamepad_Parameters.Menu_Button = true; break;
                        case 9: gamepad_Parameters.Options_Button = true; break;
                        default:
                            if (i >= 10 && i <= 17) {
                                // 拡張ボタンの検出（コントローラーのボタン数に応じて調整）
                                if (i == 10) gamepad_Parameters.Button1 = true;
                                else if (i == 11) gamepad_Parameters.Button2 = true;
                                else if (i == 12) gamepad_Parameters.Button3 = true;
                                else if (i == 13) gamepad_Parameters.Button4 = true;
                                else if (i == 14) gamepad_Parameters.Button5 = true;
                                else if (i == 15) gamepad_Parameters.Button6 = true;
                                else if (i == 16) gamepad_Parameters.Button7 = true;
                                else if (i == 17) gamepad_Parameters.Button8 = true;
                            }
                            break;
                        }
                    }
                }

                // D-Pad の処理
                if (js.rgdwPOV[0] != -1) {
                    int pov = js.rgdwPOV[0] / 100;
                    if (pov == 0) gamepad_Parameters.DPad_Up = true;
                    else if (pov == 4500) gamepad_Parameters.DPad_Right = true;
                    else if (pov == 9000) gamepad_Parameters.DPad_Down = true;
                    else if (pov == 13500) gamepad_Parameters.DPad_Left = true;
                }
            }
            else
            {
                // XInput を使用して Xbox コントローラーの入力を読み取る
                XINPUT_STATE state;
                ZeroMemory(&state, sizeof(XINPUT_STATE));
                //VibrateController(0, 0);

                // コントローラーの状態を取得
                DWORD dwResult = XInputGetState(0, &state);
                if (dwResult == ERROR_SUCCESS) {
                    // スティックとトリガーの値を取得・処理する
                    // ロックを取得してコントローラー状態を更新
                    std::lock_guard<std::mutex> lock(stateMutex);

                    gamepad_Parameters.leftThumb.X = new_thuml(state.Gamepad.sThumbLX / stickSensitivity); // 左スティックX値
                    gamepad_Parameters.leftThumb.Y = new_thuml(state.Gamepad.sThumbLY / stickSensitivity); // 左スティックY値
                    gamepad_Parameters.rightThumb.X = new_thuml(state.Gamepad.sThumbRX / stickSensitivity);// 右スティックX値
                    gamepad_Parameters.rightThumb.Y = new_thuml(state.Gamepad.sThumbRY / stickSensitivity);// 右スティックY値

                    gamepad_Parameters.leftTrigger = (int)state.Gamepad.bLeftTrigger;   // 左トリガーの値
                    gamepad_Parameters.rightTrigger = (int)state.Gamepad.bRightTrigger; // 右トリガーの値



                    buttonStates = state.Gamepad.wButtons;
                    pressedButtons = GetPressedButtons(buttonStates, gamepad_Parameters);


                    // トリガーが押された場合、振動パラメータを設定する
                    //if (true) {
                    //    // 設定された振動パラメータに基づいて振動関数を呼び出す
                    //    VibrateController(state.Gamepad.bLeftTrigger, state.Gamepad.bRightTrigger);
                    //}
#ifndef GAME_DEBUG
#ifdef _DEBUG
                    gamepad_Parameters_win.text_out(pressedButtons, buttonStates, gamepad_Parameters);
#endif // DEBUG
#endif;

                }
            }
        }
#ifndef GAME_DEBUG
#ifdef _DEBUG
        gamepad_Parameters_win.text_out(pressedButtons, buttonStates, gamepad_Parameters);
#endif // DEBUG
#endif;
    }

}

// DirectInput 列挙コールバック関数。接続されているコントローラーデバイスを識別するために使用
BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext) {

    // デバイス名を出力し、コントローラーが認識されているか確認する
    name = TCHARToString(pdidInstance->tszProductName);

    // デバイスオブジェクトを作成し、検出されたコントローラーをバインドする
    if (FAILED(pDirectInput->CreateDevice(pdidInstance->guidInstance, &pGameController, NULL))) {
        return DIENUM_CONTINUE;  // 他のデバイスの列挙を継続する
    }
    return DIENUM_STOP;  // コントローラーが見つかったら列挙を停止する
}

void GamepadDetectorManager::VibrateController(WORD leftMotorSpeed, WORD rightMotorSpeed)
{

    if (Detected_Device.find("Xbox") != std::string::npos)
    {
        XINPUT_VIBRATION vibration;
        ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));

        // 振動モーターの速度を設定
        vibration.wLeftMotorSpeed = (int)((int)leftMotorSpeed / 255.0f * 65535);
        vibration.wRightMotorSpeed = (int)((int)rightMotorSpeed / 255.0f * 65535);

        // 振動設定を適用
        if (XInputSetState(0, &vibration) == ERROR_SUCCESS) {
            vibrateController = true;
        }
        else {
            vibrateController = false;
        }

    }
    else
    {
        // DirectInput による振動
        LONG forceMagnitude = (LONG)((int)leftMotorSpeed / 255.0f * DI_FFNOMINALMAX);
        VibrateWithDirectInput(pGameController, forceMagnitude);
    }
}

void GamepadDetectorManager::initialize()
{
    // DirectInput を初期化し、DirectInput オブジェクトのポインタ pDirectInput を取得する
    if (FAILED(DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&pDirectInput, NULL))) {
        MessageBox(NULL, L"Failed to initialize DirectInput.", L"BUG", MB_OK | MB_ICONERROR);
        return; // DirectInput の初期化に失敗した場合、エラーメッセージを出力して関数を終了する
    }

    // システム内の全コントローラーデバイスを列挙し、列挙コールバック関数 EnumJoysticksCallback を呼び出す
    if (FAILED(pDirectInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, NULL, DIEDFL_ATTACHEDONLY))) {
        MessageBox(NULL, L"Failed to enumerate devices.", L"BUG", MB_OK | MB_ICONERROR);
        return; // デバイスの列挙に失敗した場合、エラーメッセージを出力して関数を終了する
    }

    // コントローラーへの接続が成功した場合、データフォーマットを設定してメインループに入る
    if (pGameController) {
        Detected_Device = name;
        // コントローラーのデータフォーマットを設定する。DirectInput 定義済みフォーマット c_dfDIJoystick を使用してコントローラー入力を読み取る
        if (FAILED(pGameController->SetDataFormat(&c_dfDIJoystick))) {
            MessageBox(NULL, L"Failed to set data format.", L"BUG", MB_OK | MB_ICONERROR);
            return; // データフォーマットの設定に失敗した場合、エラーメッセージを出力して関数を終了する
        }

        // 協調レベルを設定し、コントローラー入力を現在のコンソールウィンドウと共有してバックグラウンド入力を許可する
        if (FAILED(pGameController->SetCooperativeLevel(GetConsoleWindow(), DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))) {
            MessageBox(NULL, L"Failed to set cooperative level.", L"BUG", MB_OK | MB_ICONERROR);
            return; // 協調レベルの設定に失敗した場合、エラーメッセージを出力して関数を終了する
        }

    }
    else {
        MessageBox(NULL, L"Controller not connected.", L"question", MB_OK | MB_ICONERROR);
        // リソースを解放する
        if (pGameController) pGameController->Unacquire(); // コントローラーデバイスの占有を解放し、他のプログラムが使用できるようにする
        if (pDirectInput) pDirectInput->Release(); // DirectInput オブジェクトを解放してリソースをクリーンアップする
    }



}


void GamepadDetectorManager::runControllerStateCheck() {
    while (isControllerThreadRunning.load()) {  // isControllerThreadRunning が false になるとスレッドが終了する

        GetControllerState(); // コントローラーの状態を継続的に取得してコンソールに更新する
        // ボタンが押されていない場合は sleep 時間を増やして CPU 負荷を軽減できる
        if (buttonStates == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

// ウィンドウを起動し、独立スレッドで出力を表示する
void GamepadDetectorManager::run() {

    std::thread GamepadAv(&GamepadDetectorManager::gamepadAvailable, this);
    GamepadAv.detach();


}

bool GamepadDetectorManager::IsPressed(const std::string t)
{

    if (t == "A") return gamepad_Parameters.A;
    else if (t == "B") return gamepad_Parameters.B;
    else if (t == "X") return gamepad_Parameters.X;
    else if (t == "Y") return gamepad_Parameters.Y;

    else if (t == "DPad_Up") return gamepad_Parameters.DPad_Up;
    else if (t == "DPad_Down") return gamepad_Parameters.DPad_Down;
    else if (t == "DPad_Left") return gamepad_Parameters.DPad_Left;
    else if (t == "DPad_Right") return gamepad_Parameters.DPad_Right;

    else if (t == "Left_Shoulder") return gamepad_Parameters.Left_Shoulder;
    else if (t == "Right_Shoulder") return gamepad_Parameters.Right_Shoulder;

    else if (t == "Left_Trigger_Button") return gamepad_Parameters.Left_Trigger_Button;
    else if (t == "Right_Trigger_Button") return gamepad_Parameters.Right_Trigger_Button;

    else if (t == "Button1") return gamepad_Parameters.Button1;
    else if (t == "Button2") return gamepad_Parameters.Button2;
    else if (t == "Button3") return gamepad_Parameters.Button3;
    else if (t == "Button4") return gamepad_Parameters.Button4;
    else if (t == "Button5") return gamepad_Parameters.Button5;
    else if (t == "Button6") return gamepad_Parameters.Button6;
    else if (t == "Button7") return gamepad_Parameters.Button7;
    else if (t == "Button8") return gamepad_Parameters.Button8;

    else if (t == "Menu_Button") return gamepad_Parameters.Menu_Button;
    else if (t == "Options_Button") return gamepad_Parameters.Options_Button;
    else if (t == "Left_Middle_Button") return gamepad_Parameters.Left_Middle_Button;
    else if (t == "Right_Middle_Button") return gamepad_Parameters.Right_Middle_Button;
    else if (t == "Central_Button") return gamepad_Parameters.Central_Button;

    return false;
}

bool GamepadDetectorManager::JustPressed(const std::string t)
{
    bool returnDown;
    bool old;
    bool dow;
    Gamepad_Parameters* par;
    for (int i = 0; i < 2; i++)
    {
        par = i ? &gamepad_Parameters : &old_gamepad_Parameters;

        if (t == "A") returnDown = par->A;
        else if (t == "B") returnDown = par->B;
        else if (t == "X") returnDown = par->X;
        else if (t == "Y") returnDown = par->Y;

        else if (t == "DPad_Up") returnDown = par->DPad_Up;
        else if (t == "DPad_Down") returnDown = par->DPad_Down;
        else if (t == "DPad_Left") returnDown = par->DPad_Left;
        else if (t == "DPad_Right") returnDown = par->DPad_Right;

        else if (t == "Left_Shoulder") returnDown = par->Left_Shoulder;
        else if (t == "Right_Shoulder") returnDown = par->Right_Shoulder;

        else if (t == "Left_Trigger_Button") returnDown = par->Left_Trigger_Button;
        else if (t == "Right_Trigger_Button") returnDown = par->Right_Trigger_Button;

        else if (t == "Button1") returnDown = par->Button1;
        else if (t == "Button2") returnDown = par->Button2;
        else if (t == "Button3") returnDown = par->Button3;
        else if (t == "Button4") returnDown = par->Button4;
        else if (t == "Button5") returnDown = par->Button5;
        else if (t == "Button6") returnDown = par->Button6;
        else if (t == "Button7") returnDown = par->Button7;
        else if (t == "Button8") returnDown = par->Button8;

        else if (t == "Menu_Button") returnDown = par->Menu_Button;
        else if (t == "Options_Button") returnDown = par->Options_Button;
        else if (t == "Left_Middle_Button") returnDown = par->Left_Middle_Button;
        else if (t == "Right_Middle_Button") returnDown = par->Right_Middle_Button;
        else if (t == "Central_Button") returnDown = par->Central_Button;

        (i ? dow : old) = returnDown;

    }

    par = &old_gamepad_Parameters;
    if (dow && !old)
    {
        if (t == "A") par->A = true;
        else if (t == "B") par->B = true;
        else if (t == "X") par->X = true;
        else if (t == "Y") par->Y = true;

        else if (t == "DPad_Up") par->DPad_Up = true;
        else if (t == "DPad_Down") par->DPad_Down = true;
        else if (t == "DPad_Left") par->DPad_Left = true;
        else if (t == "DPad_Right") par->DPad_Right = true;

        else if (t == "Left_Shoulder") par->Left_Shoulder = true;
        else if (t == "Right_Shoulder") par->Right_Shoulder = true;

        else if (t == "Left_Trigger_Button") par->Left_Trigger_Button = true;
        else if (t == "Right_Trigger_Button") par->Right_Trigger_Button = true;

        else if (t == "Button1") par->Button1 = true;
        else if (t == "Button2") par->Button2 = true;
        else if (t == "Button3") par->Button3 = true;
        else if (t == "Button4") par->Button4 = true;
        else if (t == "Button5") par->Button5 = true;
        else if (t == "Button6") par->Button6 = true;
        else if (t == "Button7") par->Button7 = true;
        else if (t == "Button8") par->Button8 = true;

        else if (t == "Menu_Button") par->Menu_Button = true;
        else if (t == "Options_Button") par->Options_Button = true;
        else if (t == "Left_Middle_Button") par->Left_Middle_Button = true;
        else if (t == "Right_Middle_Button") par->Right_Middle_Button = true;
        else if (t == "Central_Button") par->Central_Button = true;
        return true;
    }

    if (!dow)
    {
        if (t == "A") par->A = false;
        else if (t == "B") par->B = false;
        else if (t == "X") par->X = false;
        else if (t == "Y") par->Y = false;

        else if (t == "DPad_Up") par->DPad_Up = false;
        else if (t == "DPad_Down") par->DPad_Down = false;
        else if (t == "DPad_Left") par->DPad_Left = false;
        else if (t == "DPad_Right") par->DPad_Right = false;

        else if (t == "Left_Shoulder") par->Left_Shoulder = false;
        else if (t == "Right_Shoulder") par->Right_Shoulder = false;

        else if (t == "Left_Trigger_Button") par->Left_Trigger_Button = false;
        else if (t == "Right_Trigger_Button") par->Right_Trigger_Button = false;

        else if (t == "Button1") par->Button1 = false;
        else if (t == "Button2") par->Button2 = false;
        else if (t == "Button3") par->Button3 = false;
        else if (t == "Button4") par->Button4 = false;
        else if (t == "Button5") par->Button5 = false;
        else if (t == "Button6") par->Button6 = false;
        else if (t == "Button7") par->Button7 = false;
        else if (t == "Button8") par->Button8 = false;

        else if (t == "Menu_Button") par->Menu_Button = false;
        else if (t == "Options_Button") par->Options_Button = false;
        else if (t == "Left_Middle_Button") par->Left_Middle_Button = false;
        else if (t == "Right_Middle_Button") par->Right_Middle_Button = false;
        else if (t == "Central_Button") par->Central_Button = false;
    }

    return false;
}

bool GamepadDetectorManager::JustReleased(const std::string t) {
    bool returnDown;
    bool old;
    bool dow;
    Gamepad_Parameters* par;
    for (int i = 0; i < 2; i++)
    {
        par = i ? &gamepad_Parameters : &old_gamepad_Parameters;

        if (t == "A") returnDown = par->A;
        else if (t == "B") returnDown = par->B;
        else if (t == "X") returnDown = par->X;
        else if (t == "Y") returnDown = par->Y;

        else if (t == "DPad_Up")    returnDown = par->DPad_Up;
        else if (t == "DPad_Down")  returnDown = par->DPad_Down;
        else if (t == "DPad_Left")  returnDown = par->DPad_Left;
        else if (t == "DPad_Right") returnDown = par->DPad_Right;

        else if (t == "Left_Shoulder")  returnDown = par->Left_Shoulder;
        else if (t == "Right_Shoulder") returnDown = par->Right_Shoulder;

        else if (t == "Left_Trigger_Button")  returnDown = par->Left_Trigger_Button;
        else if (t == "Right_Trigger_Button") returnDown = par->Right_Trigger_Button;

        else if (t == "Button1") returnDown = par->Button1;
        else if (t == "Button2") returnDown = par->Button2;
        else if (t == "Button3") returnDown = par->Button3;
        else if (t == "Button4") returnDown = par->Button4;
        else if (t == "Button5") returnDown = par->Button5;
        else if (t == "Button6") returnDown = par->Button6;
        else if (t == "Button7") returnDown = par->Button7;
        else if (t == "Button8") returnDown = par->Button8;

        else if (t == "Menu_Button")         returnDown = par->Menu_Button;
        else if (t == "Options_Button")      returnDown = par->Options_Button;
        else if (t == "Left_Middle_Button")  returnDown = par->Left_Middle_Button;
        else if (t == "Right_Middle_Button") returnDown = par->Right_Middle_Button;
        else if (t == "Central_Button")      returnDown = par->Central_Button;

        (i ? dow : old) = returnDown;
    }

    par = &old_gamepad_Parameters;

    // 「前フレームは押していた」かつ「今は離している」瞬間
    if (!dow && old)
    {
        if (t == "A") par->A = false;
        else if (t == "B") par->B = false;
        else if (t == "X") par->X = false;
        else if (t == "Y") par->Y = false;

        else if (t == "DPad_Up")    par->DPad_Up = false;
        else if (t == "DPad_Down")  par->DPad_Down = false;
        else if (t == "DPad_Left")  par->DPad_Left = false;
        else if (t == "DPad_Right") par->DPad_Right = false;

        else if (t == "Left_Shoulder")  par->Left_Shoulder = false;
        else if (t == "Right_Shoulder") par->Right_Shoulder = false;

        else if (t == "Left_Trigger_Button")  par->Left_Trigger_Button = false;
        else if (t == "Right_Trigger_Button") par->Right_Trigger_Button = false;

        else if (t == "Button1") par->Button1 = false;
        else if (t == "Button2") par->Button2 = false;
        else if (t == "Button3") par->Button3 = false;
        else if (t == "Button4") par->Button4 = false;
        else if (t == "Button5") par->Button5 = false;
        else if (t == "Button6") par->Button6 = false;
        else if (t == "Button7") par->Button7 = false;
        else if (t == "Button8") par->Button8 = false;

        else if (t == "Menu_Button")         par->Menu_Button = false;
        else if (t == "Options_Button")      par->Options_Button = false;
        else if (t == "Left_Middle_Button")  par->Left_Middle_Button = false;
        else if (t == "Right_Middle_Button") par->Right_Middle_Button = false;
        else if (t == "Central_Button")      par->Central_Button = false;
        return true;
    }

    // 押されている間は old を true に保つ
    if (dow)
    {
        if (t == "A") par->A = true;
        else if (t == "B") par->B = true;
        else if (t == "X") par->X = true;
        else if (t == "Y") par->Y = true;

        else if (t == "DPad_Up")    par->DPad_Up = true;
        else if (t == "DPad_Down")  par->DPad_Down = true;
        else if (t == "DPad_Left")  par->DPad_Left = true;
        else if (t == "DPad_Right") par->DPad_Right = true;

        else if (t == "Left_Shoulder")  par->Left_Shoulder = true;
        else if (t == "Right_Shoulder") par->Right_Shoulder = true;

        else if (t == "Left_Trigger_Button")  par->Left_Trigger_Button = true;
        else if (t == "Right_Trigger_Button") par->Right_Trigger_Button = true;

        else if (t == "Button1") par->Button1 = true;
        else if (t == "Button2") par->Button2 = true;
        else if (t == "Button3") par->Button3 = true;
        else if (t == "Button4") par->Button4 = true;
        else if (t == "Button5") par->Button5 = true;
        else if (t == "Button6") par->Button6 = true;
        else if (t == "Button7") par->Button7 = true;
        else if (t == "Button8") par->Button8 = true;

        else if (t == "Menu_Button")         par->Menu_Button = true;
        else if (t == "Options_Button")      par->Options_Button = true;
        else if (t == "Left_Middle_Button")  par->Left_Middle_Button = true;
        else if (t == "Right_Middle_Button") par->Right_Middle_Button = true;
        else if (t == "Central_Button")      par->Central_Button = true;
    }

    return false;
}

bool GamepadDetectorManager::IsAnyInput() {
    if (IsPressed("A")) return true;
    if (IsPressed("B")) return true;
    if (IsPressed("X")) return true;
    if (IsPressed("Y")) return true;
    if (IsPressed("DPad_Up")) return true;
    if (IsPressed("DPad_Down")) return true;
    if (IsPressed("DPad_Left")) return true;
    if (IsPressed("DPad_Right")) return true;
    if (IsPressed("Left_Shoulder"))      return true;
    if (IsPressed("Right_Shoulder"))     return true;

    // スティック
    vector2 left = GetleftThumb();
    vector2 right = GetrightThumb();
    if (left.X != 0 || left.Y != 0)           return true;
    if (right.X != 0 || right.Y != 0)           return true;

    // トリガー
    if (GetleftTrigger() > 0)           return true;
    if (GetrightTrigger() > 0)           return true;

    return false;
}

vector2& GamepadDetectorManager::GetleftThumb(void)
{
    return gamepad_Parameters.leftThumb;
}

vector2& GamepadDetectorManager::GetrightThumb(void)
{
    return gamepad_Parameters.rightThumb;
}

int& GamepadDetectorManager::GetleftTrigger(void)
{
    return gamepad_Parameters.leftTrigger;
}

int& GamepadDetectorManager::GetrightTrigger(void)
{
    return gamepad_Parameters.rightTrigger;
}

float GamepadDetectorManager::GetleftThumbAngle(void)
{
    float x, y;
    x = -gamepad_Parameters.leftThumb.X;
    y = gamepad_Parameters.leftThumb.Y;

    if (x == 0.0f && y == 0.0f)
        return -1.0f;   // 中心。方向なし

    float angle = atan2f(x, y);   // [-pi, +pi]
    return angle;                // ラジアン
}

float GamepadDetectorManager::GetleftThumbAngleDeg(void)
{
    float x, y;
    x = gamepad_Parameters.leftThumb.X;
    y = gamepad_Parameters.leftThumb.Y;

    if (x == 0.0f && y == 0.0f)
        return -1.0f;

    float angle = atan2f(x, y) * 180.0f / 3.1415926535f;

    if (angle < 0.0f)
        angle += 360.0f;   // 0〜360 に変換

    return angle;
}

float GamepadDetectorManager::GetrightThumbAngle(void)
{
    float x, y;
    x = gamepad_Parameters.rightThumb.X;
    y = gamepad_Parameters.rightThumb.Y;

    if (x == 0.0f && y == 0.0f)
        return -1.0f;   // 中心。方向なし

    float angle = atan2f(x, y);   // [-pi, +pi]
    return angle;                // ラジアン
}

float GamepadDetectorManager::GetrightThumbAngleDeg(void)
{
    float x, y;
    x = gamepad_Parameters.rightThumb.X;
    y = gamepad_Parameters.rightThumb.Y;

    if (x == 0.0f && y == 0.0f)
        return -1.0f;

    float angle = atan2f(x, y) * 180.0f / 3.1415926535f;

    if (angle < 0.0f)
        angle += 360.0f;   // 0〜360 に変換

    return angle;
}


void GamepadDetectorManager::gamepadAvailable() {

    bool wasConnected = false;  // 前回の接続状態を記録する

    while (1)
    {
        XINPUT_STATE state;
        ZeroMemory(&state, sizeof(XINPUT_STATE));

        DWORD result = XInputGetState(0, &state);  // 1P コントローラー（インデックス 0）のみ検出

        if (result == ERROR_SUCCESS && !wasConnected)
        {


            initialize();
#ifndef GAME_DEBUG
#ifdef _DEBUG
            if (!isWindowThreadRunning.load()) {  // 未起動の場合のみウィンドウスレッドを起動する
                //MessageBox(NULL, L"no off", L"BUG", MB_OK | MB_ICONERROR);
                isWindowThreadRunning.store(true);
                std::thread windowThread(&createWindow::run, &gamepad_Parameters_win);
                windowThread.detach();
            }
#endif // DEBUG
#endif
            if (!isControllerThreadRunning.load())
            {
                isControllerThreadRunning.store(true);
                // コントローラー状態取得スレッドを起動し、現在のオブジェクトのポインタを渡す
                std::thread controllerThread(&GamepadDetectorManager::runControllerStateCheck, this);
                controllerThread.detach(); // スレッドを独立して実行させる。メインスレッドはその終了を待たない
            }

        }
        else if (result != ERROR_SUCCESS && wasConnected)  // コントローラーが切断された
        {
            isControllerThreadRunning.store(false);  // `runControllerStateCheck()` スレッドが自力で終了するようにする
#ifndef GAME_DEBUG
#ifdef _DEBUG
            isWindowThreadRunning.store(false);  // ウィンドウスレッドも停止させる
#endif // DEBUG
#endif;
        }

        wasConnected = (result == ERROR_SUCCESS);  // 現在の接続状態を記録する

        std::this_thread::sleep_for(std::chrono::seconds(2));  // 2秒ごとに確認し、CPU過負荷を防ぐ

    }

}

void VibrateWithDirectInput(IDirectInputDevice8* pGameController, LONG magnitude)
{
    if (!pGameController) return;

    DIEFFECT eff;
    ZeroMemory(&eff, sizeof(DIEFFECT));
    eff.dwSize = sizeof(DIEFFECT);
    eff.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
    eff.dwDuration = INFINITE; // 振動の持続時間
    eff.dwGain = DI_FFNOMINALMAX; // 振動強度
    eff.dwTriggerButton = DIEB_NOTRIGGER;
    eff.cAxes = 1; // 1軸のみに影響（振動）
    eff.rgdwAxes = NULL;
    eff.rglDirection = NULL;
    eff.lpEnvelope = NULL;
    eff.cbTypeSpecificParams = sizeof(DICONSTANTFORCE);
    eff.lpvTypeSpecificParams = &magnitude;

    IDirectInputEffect* pEffect = nullptr;
    HRESULT hr = pGameController->CreateEffect(GUID_ConstantForce, &eff, &pEffect, NULL);
    if (SUCCEEDED(hr))
    {
        pEffect->Start(1, 0); // 振動を開始する
    }
}


