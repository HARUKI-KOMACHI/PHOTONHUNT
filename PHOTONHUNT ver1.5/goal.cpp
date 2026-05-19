#include "goal.h"
#include "texture.h"
#include "main.h"
#include "collision.h"
#include "player_1.h"

Goal g_goal;

unsigned int goalCloseTexNo = -1;
unsigned int goalOpenTexNo = -1;

void InitializeGoal() {
	goalCloseTexNo = LoadTexture("Assets/Textures/objects/goal_close.png");
	goalOpenTexNo = LoadTexture("Assets/Textures/objects/goal_open.png");

	g_goal.obj.showPolyCollision = false;
	g_goal.isOpen = false;
}

void FinalizeGoal() {
	UnloadTexture(goalCloseTexNo);
	UnloadTexture(goalOpenTexNo);

	g_goal.obj.vt.clear();
	g_goal.obj.Indices_vt.clear();
}

void UpdateGoal() {
	if (Win->kbd.KeyJustPressed('U')) {
		g_goal.obj.showPolyCollision = !g_goal.obj.showPolyCollision;
	}

	if (g_goal.isOpen) {
		g_goal.obj.texNo = goalOpenTexNo;
	}
	else {
		g_goal.obj.texNo = goalCloseTexNo;
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

	if (g_goal.obj.vt.empty())return;

	// vtから最上部の頂点を探す（UV座標→ワールド座標に変換）
	float maxY = -FLT_MAX;
	float g_goalminX = FLT_MAX;
	float g_goalmaxX = -FLT_MAX;

	for (const auto& v : g_goal.obj.vt) {
		float worldX = g_goal.obj.Transform.Position.x + (v.x - 0.5f) * g_goal.obj.Size.x;
		float worldY = g_goal.obj.Transform.Position.y + (v.y - 0.5f) * g_goal.obj.Size.y;

		if (worldY > maxY) {
			maxY = worldY;
		}
		if (worldX < g_goalminX) {
			g_goalminX = worldX;
		}
		if (worldX > g_goalmaxX) {
			g_goalmaxX = worldX;
		}
	}

	// プレイヤーがX範囲内 & Y距離が近い
	bool inXRange = !(playerMaxX < g_goalminX || playerMinX > g_goalmaxX);
	float yDistance = player.anim->obj.Transform.Position.y - maxY;
	bool alpha = (yDistance > 0 && yDistance < 50.0f);
	bool Zorder = (yDistance > 0 && yDistance < 200.0f);

	if (inXRange && alpha)
	{
		g_goal.obj.Z_Test = 0.7f;
	}
	else if (inXRange && Zorder)
	{
		g_goal.obj.Z_Test = 0.7f;
	}
	else
	{
		g_goal.obj.Z_Test = 0.5f;
	}
}

void DrawGoal() {
	if (g_goal.obj.texNo != -1) {
		BoxVertexsMake(&g_goal.obj);
		DrawObject_2D(&g_goal.obj);
	}
}

void AddGoal(float x, float y, float w, float h, const std::string& objFile) {

	g_goal.obj.Transform.Position.x = x;
	g_goal.obj.Transform.Position.y = y;
	g_goal.obj.Transform.Rotation = 0.0f;
	g_goal.obj.Transform.Scale.x = 1.0f;
	g_goal.obj.Transform.Scale.y = 1.0f;
	g_goal.obj.Size.x = w;
	g_goal.obj.Size.y = h;
	g_goal.obj.flipX = false;
	g_goal.obj.flipY = false;
	g_goal.obj.Z_Test = 0.4f;
	g_goal.obj.showPolyCollision = false;

	if (!objFile.empty()) {
		g_goal.obj.loadPolyCollision(objFile);
	}

	// OBJのポリゴンコリジョンのバッファを事前生成
	if (!g_goal.obj.Indices_vt.empty())
	{
		std::vector<Object_2D::Vec2> vt;
		std::vector<Vector> vx;
		ConvertUVtoModel(g_goal.obj, &vt);

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
		Win->Gfx().pDevice->CreateBuffer(&vbd, &vd, g_goal.obj.cachedVertexBuffer.GetAddressOf());

		D3D11_BUFFER_DESC ibd = {};
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.Usage = D3D11_USAGE_DEFAULT;
		ibd.ByteWidth = static_cast<UINT>(g_goal.obj.Indices_vt.size() * sizeof(uint16_t));
		ibd.StructureByteStride = sizeof(uint16_t);
		D3D11_SUBRESOURCE_DATA id = {};
		id.pSysMem = g_goal.obj.Indices_vt.data();
		Win->Gfx().pDevice->CreateBuffer(&ibd, &id, g_goal.obj.cachedIndexBuffer.GetAddressOf());

		g_goal.obj.cachedIndexCount = (int)g_goal.obj.Indices_vt.size();
		g_goal.obj.bufferReady = true;
	}
}