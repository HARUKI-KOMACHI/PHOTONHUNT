#pragma once


#include	<sdkddkver.h>
#define		WIN32_LEAN_AND_MEAN

#include	"ChiliWin.h"
#include	"debug_ostream.h"

//追加
#include	<algorithm>
#include	"shader.h"
#include	"sprite.h"
#include	"sprite_3D.h"
#include	"DX_Spine.h"
//

#include	"GamepadManager _1.0/GamepadManager.h"
#include	"GamepadManager _1.0/GamepadManager_DEDUG.h"

#include	"Keyboard.h"
#include	"Mouse.h"

#include "imgui.h"
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>
#include <imgui_internal.h>
#include "Window.h"



#include <vector>

#include <memory>


#include    <string>
#include    <d3d11.h>
#include    <wrl.h>
#include    <optional>
#include    <sstream>
#include    <d3dcompiler.h>
#include    <cmath>
#include    <DirectXMath.h>
#include    <iostream>

#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <shellapi.h>   // DragQueryFileW / HDROP


#pragma comment(lib,"d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Shell32.lib")


void UninitSystem();



extern GamepadDetectorManager manager;


extern std::unique_ptr<Window> Win;

extern std::vector<Object_2D*> object_2D_list_Darw;
extern std::vector<Object_2D*> collision_2D_list_Draw;

extern std::vector<Object_3D*> object_3D_list_Darw_Opaque;
extern std::vector<Object_3D*>	object_3D_list_Darw_Transparent;

struct SkinningCB
{
    DirectX::XMFLOAT4X4 Bones[540];
    int HasSkin;
    float padding[3];
};

