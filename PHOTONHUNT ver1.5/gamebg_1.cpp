#include "gamebg_1.h"
#include "texture.h"
#include "Editor.h"
#include "collision.h"
#include "main.h"

GameBG g_Gamebg;
Object_2D stair(0, 0, 300, 130);

Object_2D bg(0, 0, SCREEN_WIDTH * 1.5f, SCREEN_HEIGHT * 1.5f);

void InitializeGamebg01() {
	bg.texNo = LoadTexture("rom/color/bg.png");
	bg.Z_Test = -2.0f; // 背景より手前に配置

	if (CheckScene() == SCENE_GAME_1) {
		stair.Transform.Position.x = 480.0f;
		stair.Transform.Position.y = 240.0f;
		stair.texNo = LoadTexture("Assets/Textures/objects/1-1_object_stair.png");
		stair.Z_Test = 0.0f;
	}
}
void UpdateGamebg01() {
	if (Win->kbd.KeyJustPressed('I'))
	{
		g_Gamebg.obj.showPolyCollision = !g_Gamebg.obj.showPolyCollision;
	}
}
void DrawGamebg01() {
	g_Gamebg.obj.stencilRole=StencilRole::FloorWrite;
	BoxVertexsMake(&g_Gamebg.obj);
	DrawObject_2D(&g_Gamebg.obj);

	if (stair.texNo != -1) {
		BoxVertexsMake(&stair);
		DrawObject_2D(&stair);
	}
	
	if (bg.texNo != -1) {
		BoxVertexsMake(&bg);
		DrawObject_2D(&bg);
	}
}

void FinalizeGamebg01() {
	g_Gamebg.obj.vt.clear();
	g_Gamebg.obj.Indices_vt.clear();

	stair.texNo = -1;
	bg.texNo = -1;
}

void AddBG(const std::string& textureFile, const std::string& objFile) {
	g_Gamebg.obj.Transform.Position.x = 0.0f;
	g_Gamebg.obj.Transform.Position.y = 0.0f;
	g_Gamebg.obj.Transform.Rotation = 0.0f;
	g_Gamebg.obj.Transform.Scale.x = 1.0f;
	g_Gamebg.obj.Transform.Scale.y = 1.0f;
	g_Gamebg.obj.Size.x = SCREEN_WIDTH; // デフォルトサイズ
	g_Gamebg.obj.Size.y = SCREEN_HEIGHT; // デフォルトサイズ
	g_Gamebg.obj.flipX = false;
	g_Gamebg.obj.flipY = false;
	g_Gamebg.obj.Z_Test = -1.0f;
	g_Gamebg.obj.showPolyCollision = false;

	// テクスチャをロード
	if (!textureFile.empty())
	{
		Object_2D::Vec2 texSize;
		g_Gamebg.obj.texNo = LoadTexture(textureFile, texSize);
		// テクスチャサイズが指定されていない場合、画像サイズを使用
		if (g_Gamebg.obj.Size.x == 0 && g_Gamebg.obj.Size.y == 0)
		{
			g_Gamebg.obj.Size.x = texSize.x;
			g_Gamebg.obj.Size.y = texSize.y;
		}
	}
	else
	{
		// デフォルトテクスチャを使用
		g_Gamebg.obj.texNo = LoadTexture("rom/color/white.png");
	}
	if (!objFile.empty())
	{
		g_Gamebg.obj.loadPolyCollision(objFile);
	}

	// OBJのポリゴンコリジョンのバッファを事前生成
	if (!g_Gamebg.obj.Indices_vt.empty())
	{
		std::vector<Object_2D::Vec2> vt;
		std::vector<Vector> vx;
		ConvertUVtoModel(g_Gamebg.obj, &vt);

		for (auto& v : vt) {
			Vector vv;
			vv.position = { v.x, v.y, 0, 1 };
			vv.color = { 1,1,1,1 };
			vv.texcoord = { 0,0 };
			vv.normal = { 0,0,0 };
			vv.hasTexture = 0;
			vx.push_back(vv);
		}

		D3D11_BUFFER_DESC vbd = {};
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.Usage = D3D11_USAGE_DEFAULT;
		vbd.ByteWidth = static_cast<UINT>(vx.size() * sizeof(Vector));
		D3D11_SUBRESOURCE_DATA vd = {};
		vd.pSysMem = vx.data();
		Win->Gfx().pDevice->CreateBuffer(&vbd, &vd, g_Gamebg.obj.cachedVertexBuffer.GetAddressOf());

		D3D11_BUFFER_DESC ibd = {};
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.Usage = D3D11_USAGE_DEFAULT;
		ibd.ByteWidth = static_cast<UINT>(g_Gamebg.obj.Indices_vt.size() * sizeof(uint16_t));
		ibd.StructureByteStride = sizeof(uint16_t);
		D3D11_SUBRESOURCE_DATA id = {};
		id.pSysMem = g_Gamebg.obj.Indices_vt.data();
		Win->Gfx().pDevice->CreateBuffer(&ibd, &id, g_Gamebg.obj.cachedIndexBuffer.GetAddressOf());

		g_Gamebg.obj.cachedIndexCount = (int)g_Gamebg.obj.Indices_vt.size();
		g_Gamebg.obj.bufferReady = true;
	}
}

GameBG& GetGameBG() {
	return g_Gamebg;
}