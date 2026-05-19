#include "sprite.h"


#include "main.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include "texture.h"

#include "shader.h"
#include "debug_ostream.h"


unsigned int	g_SpriteVertexArrayObject;
unsigned int	g_SpriteVertexBuffer;

Float2 ScreenOffset;


static constexpr int NUM_VERTEX = 6;

void DrawObject_2D(Object_2D* obj_2D)
{
	if (obj_2D->bShow)
	{
		object_2D_list_Darw.push_back(obj_2D);
	}

	if (obj_2D->showPolyCollision)
	{
		collision_2D_list_Draw.push_back(obj_2D);
	}
}


//void UninitSprite()
//{
//
//	if (GetTexture().size() != 0)
//	{
//		for (int i = 0;i < GetTexture().size();i++)
//		{
//			GetTexture()[i]->Release();
//		}
//	}
//	GetTexture().clear();
//	if (g_pConstantBuffer)
//	{
//		g_pConstantBuffer->Release();
//	}
//
//}

void UninitSprite()
{
	auto& textures = GetTexture();
	textures.clear();

	if (g_pConstantBuffer)
	{
		g_pConstantBuffer->Release();
		g_pConstantBuffer = nullptr;
	}
}




void InitializeScreenOffset(void)
{
	ScreenOffset = MakeFloat2(0.0f, 0.0f);
}


void SetScreenOffset(float x, float y)
{

	ScreenOffset.x += x;
	ScreenOffset.y -= y;
	/*if (ScreenOffset.x > 0)
	{
		ScreenOffset.x = 0.0f;
	}*/
}

float GetScreenOffsetX(void)
{
	return ScreenOffset.x;
}

float GetScreenOffsetY(void)
{
	return ScreenOffset.y;
}
Object_2D::Object_2D()
{
	Transform.Position.x = 0;
	Transform.Position.y = 0;

	Size.x = 0;
	Size.y = 0;

	UV[0] = { 0.0f,0.0f };
	UV[1] = { 1.0f,0.0f };
	UV[2] = { 0.0f,1.0f };
	UV[3] = { 1.0f,1.0f };
}
Object_2D::Object_2D(float x, float y, float w, float h)
{
	Transform.Position.x = x;
	Transform.Position.y = y;

	Size.x = w;
	Size.y = h;

	Vertexs[0] = { Transform.Position.x - Size.x / 2, Transform.Position.y - Size.y / 2};	//左上
	Vertexs[1] = { Transform.Position.x + Size.x / 2, Transform.Position.y - Size.y / 2};	//右上
	Vertexs[2] = { Transform.Position.x - Size.x / 2, Transform.Position.y + Size.y / 2};	//左下
	Vertexs[3] = { Transform.Position.x + Size.x / 2, Transform.Position.y + Size.y / 2};	//右下

	UV[0] = { 0.0f,0.0f };
	UV[1] = { 1.0f,0.0f };
	UV[2] = { 0.0f,1.0f };
	UV[3] = { 1.0f,1.0f };

}



void Object_2D::loadPolyCollision(std::string Path)
{
	std::filesystem::path filePath(Path);

	std::string line;
	std::ifstream file(Path);



	while (getline(file, line))
	{
		std::stringstream s;
		s.clear();   // 清理流
		s.str("");   // 清空流
		s << line;

		char junk;
		int ty = 0;
		if (line[0] == 'v')
		{
			Object_2D::Vec2 vertext;
			if (line[1] == 't') // 处理纹理坐标
			{
				s >> junk >> junk >> vertext.x >> vertext.y;
				vt.push_back(vertext);

				printf("vt[%d] = (%.6f, %.6f)\n", vertext.x, vertext.y);
			}

		}

		if (line[0] == 'f')
		{
			for (auto a : line)
			{
				if (a == ' ')
				{
					ty++;
				}
			}

			if (ty == 3)
			{
				int f[3][3];
				s >> junk >>
					f[0][0] >> junk >> f[0][1] >> junk >> f[0][2] >>
					f[1][0] >> junk >> f[1][1] >> junk >> f[1][2] >>
					f[2][0] >> junk >> f[2][1] >> junk >> f[2][2];
				Indices_vt.push_back(f[0][1] - 1);
				Indices_vt.push_back(f[1][1] - 1);
				Indices_vt.push_back(f[2][1] - 1);
			}
			else if (ty == 4)
			{
				int f[4][3];
				s >> junk >>
					f[0][0] >> junk >> f[0][1] >> junk >> f[0][2] >>
					f[1][0] >> junk >> f[1][1] >> junk >> f[1][2] >>
					f[2][0] >> junk >> f[2][1] >> junk >> f[2][2] >>
					f[3][0] >> junk >> f[3][1] >> junk >> f[3][2];
				Indices_vt.push_back(f[0][1] - 1);
				Indices_vt.push_back(f[1][1] - 1);
				Indices_vt.push_back(f[2][1] - 1);
				Indices_vt.push_back(f[0][1] - 1);
				Indices_vt.push_back(f[2][1] - 1);
				Indices_vt.push_back(f[3][1] - 1);
			}
		}
	}

	file.close();

	printf("\nIndices_vt: ");
	for (auto idx : Indices_vt)
	{
		printf("%d ", idx);
	}
	printf("\n");
}


void Object_2D_UV(Object_2D* obj_2D,int bno, int wc, int hc)
{
#define	YOKO (10)
#define TEX_w (1.0f / YOKO)

	float w = 1.0f / wc;
	float h = 1.0f / hc;

	float x = (bno % wc) * w;
	float y = (bno / wc) * h;

	obj_2D->UV[0]= { x,y };
	obj_2D->UV[1]= { x + w,y };
	obj_2D->UV[2]= { x,y + h };
	obj_2D->UV[3]= { x + w,y + h };
}


void BoxVertexsMake(Object_2D* obj_2D)
{
	// 物体尺寸的一半
	float halfW = obj_2D->Size.x * 0.5f;
	float halfH = obj_2D->Size.y * 0.5f;

	// 局部坐标（以中心为原点）
	Object_2D::Vec2 local[4] =
	{
		{ -halfW,  halfH },   // 左上
		{  halfW,  halfH },   // 右上
		{ -halfW, -halfH },   // 左下
		{  halfW, -halfH }    // 右下
	};

	// ======================
	// 旋转 + 缩放计算
	// ======================
	float c = cosf(obj_2D->Transform.Rotation);
	float s = sinf(obj_2D->Transform.Rotation);

	for (int i = 0; i < 4; ++i)
	{
		// 缩放
		float x = local[i].x * obj_2D->Transform.Scale.x;
		float y = local[i].y * obj_2D->Transform.Scale.y;

		// 旋转
		float rx = x * c - y * s;
		float ry = x * s + y * c;

		// 平移到世界坐标
		obj_2D->Vertexs[i].x = rx + obj_2D->Transform.Position.x + obj_2D->Transform.Offset.x;
		obj_2D->Vertexs[i].y = ry + obj_2D->Transform.Position.y + obj_2D->Transform.Offset.y;
	}
}