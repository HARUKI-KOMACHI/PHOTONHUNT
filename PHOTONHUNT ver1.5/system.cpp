#include "main.h"


#define		CLASS_NAME "DX21 Window"
#define		WINDOW_CAPTION "DX21ウィンド表示"

MSG msg;

GamepadDetectorManager manager;
std::vector<Object_2D*> object_2D_list_Darw;
std::vector<Object_2D*> collision_2D_list_Draw;


std::vector<Object_3D*> object_3D_list_Darw_Opaque;

std::vector<Object_3D*>	object_3D_list_Darw_Transparent;



void UninitSystem()
{
	Shader_Finalize();

}




