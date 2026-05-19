#pragma once
#include "ChiliWin.h"
#include "WindowState.h"
#include <wtypes.h>
#include <string>
#include <optional>
#include "Keyboard.h"
#include "Mouse.h"
#include "Graphics.h"
#include "Graphics-OpenGL.h"


class Window
{
private:
    
    class WindowClass
    {
    public:

        static const wchar_t* GetName() noexcept;

        static HINSTANCE GetInstance() noexcept;
    private:

        WindowClass() noexcept;

        ~WindowClass();

        WindowClass(const WindowClass&) = delete;
        WindowClass& operator = (const WindowClass&) = delete;
    
        static constexpr const wchar_t* wndClassName = L"Chili Direct3D Engine Window";

        static WindowClass wndClass;

        HINSTANCE hInst;
    };
    
public:
    
    Window(int Width, int Height, const wchar_t* name) noexcept;
    
    ~Window();
    
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    
    void SetTitle(const std::wstring& title);
    
    static std::optional<int> ProcessMessages() noexcept;
    
    Graphics& Gfx();
    
private:
    
    static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
    
    
    static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
    
    
    LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

public:
    Keyboard kbd;  
    Mouse mouse; 
    int width;  
    int height;   
    std::unique_ptr<Graphics_GL> pGfx_GL;
    
    WindowState winstate;
private:
    bool cursorEnabled = true; 
    HWND hWnd; 
    std::unique_ptr<Graphics> pGfx;
};




struct MonitorInfo
{
    int width;     
    int height;    
    int x;         
    int y;         
    float dpiScale;
};





BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);

void GetAllMonitors();

extern MSG Msg;

