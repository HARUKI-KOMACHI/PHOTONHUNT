#include "GamepadManager_DEDUG.h"

#ifndef GAME_DEBUG
#ifdef _DEBUG
std::atomic<bool> isWindowThreadRunning = false;
#endif
#endif
// 窗口过程函数，用于处理窗口消息
LRESULT CALLBACK createWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    createWindow* window;
    if (uMsg == WM_NCCREATE) {
        window = static_cast<createWindow*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    }
    else {
        window = reinterpret_cast<createWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (window) {
        return window->handleMessage(hwnd, uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


LRESULT createWindow::handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 设置字体
        HFONT hFont = CreateFont(
            24,           // 字体高度
            0,                  // 字体宽度（0 表示自动调整）
            0,                  // 文字的倾斜角度
            0,                  // 基线方向的倾斜角度
            FW_NORMAL,          // 字体的粗细（FW_BOLD 表示粗体）
            FALSE,              // 是否斜体
            FALSE,              // 是否下划线
            FALSE,              // 是否删除线
            DEFAULT_CHARSET,    // 字符集
            OUT_OUTLINE_PRECIS, // 输出精度
            CLIP_DEFAULT_PRECIS,// 裁剪精度
            CLEARTYPE_QUALITY,  // 输出质量
            VARIABLE_PITCH,     // 字体间距
            L"Consolas"    // 字体名称
        );

        HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

        // 获取窗口的大小
        RECT rect;
        GetClientRect(hwnd, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        // 创建一个内存中的DC（设备上下文）来进行双缓冲
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        // 使用白色填充背景
        HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(memDC, &rect, whiteBrush);
        DeleteObject(whiteBrush);

        // 在内存DC中绘制文本内容
        drawTextLineByLine(memDC);

        // 将内存中的内容复制到窗口的设备上下文中
        BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

        // 清理资源
        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);
        SelectObject(hdc, oldFont);
        DeleteObject(hFont); // 删除字体对象，释放资源

        EndPaint(hwnd, &ps);
    } break;
    case WM_CLOSE:
#ifndef GAME_DEBUG
#ifdef _DEBUG
        //窗口手动关闭，设置 isWindowThreadRunning = false
        isWindowThreadRunning.store(false);  // **手动关闭窗口时，修改全局变量**
#endif
#endif
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// 绘制文本内容，逐行输出
void createWindow::drawTextLineByLine(HDC hdc) {
    int lineHeight = 20; // 行高
    int x = 10, y = 10;  // 初始绘制坐标
    std::lock_guard<std::mutex> lock(outputMutex);
    size_t startPos = 0, endPos;
    int currentY = y;

    // 逐行绘制输出内容
    while ((endPos = output.find(L'\n', startPos)) != std::wstring::npos) {
        std::wstring line = output.substr(startPos, endPos - startPos);
        TextOutW(hdc, x, currentY, line.c_str(), line.length());
        currentY += lineHeight;
        startPos = endPos + 1;
    }

    // 绘制最后一行（如果没有以 '\n' 结尾）
    std::wstring line = output.substr(startPos);
    TextOutW(hdc, x, currentY, line.c_str(), line.length());
}

void createWindow::text_out(std::string pressedButtons, WORD buttonStates, Gamepad_Parameters Controller)
{
    std::wstringstream ss;

    // 添加按键状态信息
    ss << L"Buttons: " << std::wstring(pressedButtons.begin(), pressedButtons.end()) << L"\n";
    ss << L"Buttons value: " << buttonStates << L"\n";

    // 添加摇杆状态
    ss << L"Left thumbstick X: " << Controller.leftThumb.X << L"\n";
    ss << L"Left thumbstick Y: " << Controller.leftThumb.Y << L"\n";
    ss << L"Right thumbstick X: " << Controller.rightThumb.X << L"\n";
    ss << L"Right thumbstick Y: " << Controller.rightThumb.Y << L"\n";

    // 添加触发键状态
    ss << L"Left trigger: " << Controller.leftTrigger << L"\n";
    ss << L"Right trigger: " << Controller.rightTrigger << L"\n";

    // 添加具体按键状态（0/1，表示按下与否）
    ss << L"A: " << (Controller.A ? L"1" : L"0") << L"\n";
    ss << L"B: " << (Controller.B ? L"1" : L"0") << L"\n";
    ss << L"X: " << (Controller.X ? L"1" : L"0") << L"\n";
    ss << L"Y: " << (Controller.Y ? L"1" : L"0") << L"\n";
    ss << L"DPad_Up: " << (Controller.DPad_Up ? L"1" : L"0") << L"\n";
    ss << L"DPad_Down: " << (Controller.DPad_Down ? L"1" : L"0") << L"\n";
    ss << L"DPad_Left: " << (Controller.DPad_Left ? L"1" : L"0") << L"\n";
    ss << L"DPad_Right: " << (Controller.DPad_Right ? L"1" : L"0") << L"\n";
    ss << L"Left_Shoulder: " << (Controller.Left_Shoulder ? L"1" : L"0") << L"\n";
    ss << L"Right_Shoulder: " << (Controller.Right_Shoulder ? L"1" : L"0") << L"\n";

    // 显示扩展按键状态
    ss << L"Menu_Button: " << (Controller.Menu_Button ? L"1" : L"0") << L"\n";
    ss << L"Options_Button: " << (Controller.Options_Button ? L"1" : L"0") << L"\n";
    ss << L"Button1: " << (Controller.Button1 ? L"1" : L"0") << L"\n";
    ss << L"Button2: " << (Controller.Button2 ? L"1" : L"0") << L"\n";
    ss << L"Button3: " << (Controller.Button3 ? L"1" : L"0") << L"\n";
    ss << L"Button4: " << (Controller.Button4 ? L"1" : L"0") << L"\n";
    ss << L"Button5: " << (Controller.Button5 ? L"1" : L"0") << L"\n";
    ss << L"Button6: " << (Controller.Button6 ? L"1" : L"0") << L"\n";
    ss << L"Button7: " << (Controller.Button7 ? L"1" : L"0") << L"\n";
    ss << L"Button8: " << (Controller.Button8 ? L"1" : L"0") << L"\n";

    output = ss.str();
    if (hwnd) {
        InvalidateRect(hwnd, NULL, TRUE); // 请求窗口重绘
    }
}

void createWindow::run()
{
    const wchar_t* CLASS_NAME = L"CustomConsoleWindowClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Custom Output Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 800,
        NULL, NULL, GetModuleHandle(NULL), this
    );

    if (hwnd == NULL) {
        return;
    }

    ShowWindow(hwnd, SW_SHOW);
    // 进入消息循环
    MSG msg = {};
    while (1)
    {
#ifndef GAME_DEBUG
#ifdef _DEBUG
        if (!isWindowThreadRunning.load()) {
            //MessageBox(NULL, L"F窗口线程检测到关闭信号，自动退出...", L"BUG", MB_OK | MB_ICONERROR);
            //🛑 窗口线程检测到关闭信号，自动退出..."
            DestroyWindow(hwnd);
            break;
        }
#endif
#endif
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            //if (msg.message == WM_QUIT) {
            //    return;  // **如果收到 `WM_QUIT`，直接结束窗口线程**
            //}
            TranslateMessage(&msg);
            DispatchMessage(&msg); 
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));//窗体休息
    }
    
}