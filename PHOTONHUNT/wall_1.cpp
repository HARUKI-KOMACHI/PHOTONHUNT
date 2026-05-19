// wall.cpp 壁・障害物システム
#include "wall_1.h"
#include "shadow_1.h"
#include "collision.h"
#include "texture.h"
#include "editor.h"
#include "player_1.h"
#include <Windows.h>
#include <fstream>

// グローバル変数
static std::vector<Wall> g_Walls;

// 壁初期化
void InitializeWall01(void)
{
	g_Walls.clear();
}

// 壁更新
void UpdateWall01(void)
{
	if (Win->kbd.KeyJustPressed('O'))
	{
		for(auto & wall : g_Walls)
		{
			wall.obj.showPolyCollision = !wall.obj.showPolyCollision;
		}
	}

	Player& player = GetPlayer();

	// プレイヤーのvtから左右端を取得
	float playerMinX = FLT_MAX;
	float playerMaxX = -FLT_MAX;

	for (const auto& v : player.anim->obj.vt)
	{
		float worldX = player.anim->obj.Transform.Position.x + (v.x - 0.5f) * player.anim->obj.Size.x * player.anim->obj.Transform.Scale.x;
		if (worldX < playerMinX) playerMinX = worldX;
		if (worldX > playerMaxX) playerMaxX = worldX;
	}

	for (auto& wall : g_Walls) {
		if (wall.obj.vt.empty())return;

		// vtから最上部の頂点を探す（UV座標→ワールド座標に変換）
		float maxY = -FLT_MAX;
		float wallminX = FLT_MAX;
		float wallmaxX = -FLT_MAX;

		for (const auto& v : wall.obj.vt) {
			float worldX = wall.obj.Transform.Position.x + (v.x - 0.5f) * wall.obj.Size.x;
			float worldY = wall.obj.Transform.Position.y + (v.y - 0.5f) * wall.obj.Size.y;

			if (worldY > maxY) {
				maxY = worldY;
			}
			if (worldX < wallminX) {
				wallminX = worldX;
			}
			if (worldX > wallmaxX) {
				wallmaxX = worldX;
			}
		}

		// プレイヤーがX範囲内 & Y距離が近い
		bool inXRange = !(playerMaxX < wallminX || playerMinX > wallmaxX);
		float yDistance = player.anim->obj.Transform.Position.y - maxY;
		bool alpha = (yDistance > 0 && yDistance < 70.0f);
		bool Zorder = (yDistance > 0 && yDistance < 200.0f);

		if (inXRange && alpha)
		{
			wall.obj.Color.A = 0.5f;
			wall.obj.Z_Test = 0.7f;
		}
		else if (inXRange && Zorder)
		{
			wall.obj.Color.A = 1.0f;
			wall.obj.Z_Test = 0.7f;
		}
		else
		{
			wall.obj.Color.A = 1.0f;
			wall.obj.Z_Test = 0.5f;
		}
	}
}

// 壁描画
void DrawWall01(void)
{
	for (auto& wall : g_Walls)
	{
		BoxVertexsMake(&wall.obj);
		DrawObject_2D(&wall.obj);
	}
}

void FinalizeWall01() {

}

// 壁追加
void AddWall(float x, float y, float w, float h
	, const std::string& textureFile, const std::string& objFile
	, float wallHeight,float shadowX,float shadowY)
{
	Wall wall;
	wall.obj.Transform.Position.x = x;
	wall.obj.Transform.Position.y = y;
	wall.obj.Transform.Rotation = 0.0f;
	wall.obj.Transform.Scale.x = 1.0f;
	wall.obj.Transform.Scale.y = 1.0f;
	wall.obj.Size.x = w;
	wall.obj.Size.y = h;
	wall.obj.flipX = false;
	wall.obj.flipY = false;
	wall.obj.Z_Test = 0.3f;
	wall.selected = false;
	wall.createsShadow = true;
	wall.usePolygonCollision = false;
	wall.obj.showPolyCollision = false;
	wall.shadowHeight = wallHeight;
	wall.shadowFadeX = shadowX;
	wall.shadowFadeY = shadowY;

	

	// テクスチャをロード
	if (!textureFile.empty())
	{
		int texNo = LoadTexture(textureFile);
		Object_2D::Vec2 texSize;
		wall.obj.texNo = texNo;

		// テクスチャサイズが指定されていない場合、画像サイズを使用
		if (w == 0 && h == 0)
		{
			wall.obj.Size.x = texSize.x;
			wall.obj.Size.y = texSize.y;
		}
		UnloadTexture(texNo);
	}

	// OBJファイルが指定されている場合、ポリゴンデータを読み込む
	if (!objFile.empty())
	{
		wall.obj.loadPolyCollision(objFile);
		wall.usePolygonCollision = true;
	}

	// OBJのポリゴンコリジョンのバッファを事前生成
	if (wall.usePolygonCollision && !wall.obj.Indices_vt.empty())
	{
		std::vector<Object_2D::Vec2> vt;
		std::vector<Vector> vx;
		ConvertUVtoModel(wall.obj, &vt);

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
		Win->Gfx().pDevice->CreateBuffer(&vbd, &vd, wall.obj.cachedVertexBuffer.GetAddressOf());

		D3D11_BUFFER_DESC ibd = {};
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.Usage = D3D11_USAGE_DEFAULT;
		ibd.ByteWidth = static_cast<UINT>(wall.obj.Indices_vt.size() * sizeof(uint16_t));
		ibd.StructureByteStride = sizeof(uint16_t);
		D3D11_SUBRESOURCE_DATA id = {};
		id.pSysMem = wall.obj.Indices_vt.data();
		Win->Gfx().pDevice->CreateBuffer(&ibd, &id, wall.obj.cachedIndexBuffer.GetAddressOf());

		wall.obj.cachedIndexCount = (int)wall.obj.Indices_vt.size();
		wall.obj.bufferReady = true;
	}

	g_Walls.push_back(wall);
}

// 壁リスト取得
std::vector<Wall>& GetWalls(void)
{
	return g_Walls;
}