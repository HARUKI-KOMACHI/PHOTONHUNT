#include "Window.h"


std::vector<MonitorInfo> monitors;


Window::WindowClass Window::WindowClass::wndClass;


Window::WindowClass::WindowClass() noexcept
	:
	hInst(GetModuleHandle(nullptr))
{
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);       
	wc.style = CS_OWNDC;        
	wc.lpfnWndProc = HandleMsgSetup;  
	wc.cbClsExtra = 0;               
	wc.cbWndExtra = 0;              
	wc.hInstance = GetInstance();	
	wc.hIcon = nullptr;			
	wc.hCursor = nullptr;			
	wc.hbrBackground = nullptr;			
	wc.lpszMenuName = nullptr;			
	wc.lpszClassName = GetName();		
	wc.hIconSm = nullptr;			

	RegisterClassEx(&wc);            
}

Window::WindowClass::~WindowClass()
{
	UnregisterClass(wndClassName, GetInstance());
}

const wchar_t* Window::WindowClass::GetName() noexcept
{

	return wndClassName;
}


HINSTANCE Window::WindowClass::GetInstance() noexcept
{
	return wndClass.hInst;
}


Window::Window(int Width, int Height, const wchar_t* name) noexcept
	:
	width(Width),
	height(Height)
{

	GetAllMonitors();

	float dpiScale = monitors[0].dpiScale; 

	RECT wr;
	wr.left = static_cast<int>((monitors[0].width - width) / 2 / dpiScale);		
	wr.right = static_cast<int>(width + wr.left);								
	wr.top = static_cast<int>((monitors[0].height - height) / 2 / dpiScale);	
	wr.bottom = static_cast<int>(height + wr.top);								


	//AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);


	hWnd = CreateWindow(
		WindowClass::GetName(),						
		name,										
		//WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,	
		WS_POPUP,
		wr.left,									
		wr.top,										
		wr.right - wr.left,							
		wr.bottom - wr.top,							
		nullptr,									
		nullptr,									
		WindowClass::GetInstance(),					
		this										
	);


	winstate.Init(hWnd);


	pGfx = std::make_unique<Graphics>(hWnd,width,height);

	//pGfx_GL = std::make_unique<Graphics_GL>(hWnd, width, height);
}

Window::~Window()
{
	DestroyWindow(hWnd);
}

void Window::SetTitle(const std::wstring& title)
{
	SetWindowTextW(hWnd, title.c_str());

}


std::optional<int> Window::ProcessMessages() noexcept
{
	MSG msg;

	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{

		if (msg.message == WM_QUIT)
		{
			return static_cast<int>(msg.wParam);
		}

		TranslateMessage(&msg);

		DispatchMessage(&msg);
	}

	return {};
}

Graphics& Window::Gfx()
{
	return *pGfx;
}








BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	MONITORINFOEX monitorInfoEx = {};
	monitorInfoEx.cbSize = sizeof(MONITORINFOEX);

	if (GetMonitorInfo(hMonitor, &monitorInfoEx))
	{

		UINT dpi = 96; 
		HDC hdc = GetDC(nullptr);
		if (hdc)
		{
			dpi = GetDeviceCaps(hdc, LOGPIXELSX);
			ReleaseDC(nullptr, hdc);
		}

		float scaleFactor = dpi / 96.0f;


		MonitorInfo info = {
			monitorInfoEx.rcMonitor.right - monitorInfoEx.rcMonitor.left,
			monitorInfoEx.rcMonitor.bottom - monitorInfoEx.rcMonitor.top, 
			monitorInfoEx.rcMonitor.left, 
			monitorInfoEx.rcMonitor.top,  
			scaleFactor                  
		};


		std::vector<MonitorInfo>* monitors = reinterpret_cast<std::vector<MonitorInfo>*>(dwData);
		monitors->push_back(info);
	}

	return TRUE;
}


void GetAllMonitors()
{
	monitors.clear();
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors));
}


