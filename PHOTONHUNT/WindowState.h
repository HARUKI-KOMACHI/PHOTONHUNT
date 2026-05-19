#include <Windows.h>



class WindowState
{
public:

    enum class State {
        Normal,
        Minimized, 
        Maximized, 
        Hidden,   
        Showdefault
    };

    void Init(HWND winhwnd)
    {
        hWnd = winhwnd;
        ShowWindow(hWnd, SW_SHOWDEFAULT);
    }

    void Show()
    {
        ShowWindow(hWnd, SW_SHOW);
        currentState = State::Normal;
    }

    void Hide()
    {
        ShowWindow(hWnd, SW_HIDE);
        currentState = State::Hidden;
    }

    void Minimize()
    {
        ShowWindow(hWnd, SW_MINIMIZE);
        currentState = State::Minimized;
    }

    void Maximize()
    {
        ShowWindow(hWnd, SW_MAXIMIZE);
        currentState = State::Maximized;
    }

    void Showdefault()
    {
        ShowWindow(hWnd, SW_SHOWDEFAULT);
        currentState = State::Showdefault;
    }

    State GetState() const { return currentState; }

private:
    HWND hWnd;
    State currentState = State::Showdefault;
};