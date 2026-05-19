#include "shadow_1.h"
#include "player_1.h"
#include "collision.h"
#include "sun_1.h"
#include "wall_1.h"
#include "texture.h"
#include <math.h>

// グローバル変数
static std::vector<LightSource> g_Lights;
static std::vector<ShadowArea> g_Shadows;
static std::vector<DynamicShadow> g_DynamicShadows;  // 動的影
static int g_ShadowTexture = -1;  // 影のテクスチャ

// 影システム初期化
void InitializeShadow01(void)
{
	g_Lights.clear();
	g_Shadows.clear();
	g_DynamicShadows.clear();

	if (g_ShadowTexture == -1)
	{
		g_ShadowTexture = LoadTexture("rom/color/white.png");
	}
}

// 影システム更新
void UpdateShadow01(void)
{

}

// 影システム描画
void DrawShadow01(void)
{
	// 動的影を描画
	for (auto& shadow : g_DynamicShadows)
	{
		if (!shadow.active) continue;

		shadow.shadowObj.stencilRole = StencilRole::ShadowMask;
		DrawObject_2D(&shadow.shadowObj);
	}
}

void FinalizeShadow01(void)
{
	g_Lights.clear();
	g_Shadows.clear();
	g_DynamicShadows.clear();
	UnloadTexture(g_ShadowTexture);
}

// 光源追加
void AddLight(float x, float y, float radius)
{
	LightSource light;
	light.x = x;
	light.y = y;
	light.radius = radius;
	light.active = true;

	g_Lights.push_back(light);
}

// 影エリア追加
void AddShadowArea(float x, float y, float w, float h)
{
	ShadowArea shadow;
	shadow.area = Object_2D(x, y, w, h);
	shadow.active = true;

	g_Shadows.push_back(shadow);
}

// プレイヤーが影の中にいるか判定
bool CheckPlayerInShadow(void)
{
	Player& player = GetPlayer();
	if (player.anim == nullptr) return false;

	// プレイヤーの位置
	float px = player.anim->obj.Transform.Position.x;
	float py = player.anim->obj.Transform.Position.y;

	// 静的な影エリアに入っているかチェック
	for (auto& shadow : g_Shadows)
	{
		if (!shadow.active) continue;

		// 当たり判定
		if (CheckCollision_Object2D(player.anim->obj, shadow.area))
		{
			return true;
		}
	}

	// 動的影に入っているかチェック
	for (auto& shadow : g_DynamicShadows)
	{
		if (!shadow.active) continue;

		// 当たり判定
		if (CheckCollisionQuadVsMesh(shadow.shadowObj,player.anim->obj))
		{
			return true;
		}
	}

	// 光の中で、影エリアにも入っていない = 光の中
	return false;
}

// 光源リスト取得
std::vector<LightSource>& GetLights(void)
{
	return g_Lights;
}

// 影エリアリスト取得
std::vector<ShadowArea>& GetShadows(void)
{
	return g_Shadows;
}

void GenerateDynamicShadows(void)
{
	// 既存の動的影をクリア
	g_DynamicShadows.clear();

	// 太陽の方向ベクトルを取得
	Object_2D::Vec2 sunVec = GetSunVector();

	// 影の方向 = 太陽と逆方向
	Object_2D::Vec2 shadowVec;
	shadowVec.x = -sunVec.x;
	shadowVec.y = -sunVec.y;

	// 全ての壁から影を生成
	std::vector<Wall>& walls = GetWalls();

	for (auto& wall : walls)
	{
		// 影を生成しない壁はスキップ
		if (!wall.createsShadow) continue;

		// 高さが0以下の壁はスキップ
		if (wall.shadowHeight <= 0.0f) continue;

		//影の生成を制限
		if (wall.shadowFadeX >= shadowVec.x) continue;
		if (wall.shadowFadeY <= shadowVec.y) continue;

		// 壁の4頂点を取得
		Object_2D::Vec2 wallCorners[4];

		Object_2D::Vec2 shadowVerts[4];

		if (wall.usePolygonCollision && !wall.obj.vt.empty())
		{
			// OBJファイルあり: UV座標から実際の形状を取得
			float minU = 1.0f, maxU = 0.0f;
			float minV = 1.0f, maxV = 0.0f;

			int collisionCount = (int)wall.obj.vt.size() / 4;

			for (int i = 0; i < collisionCount; i++) {
				for (int j = 0; j < 4; j++) {
					float u = wall.obj.vt[(i * 4) + j].x;
					float v = wall.obj.vt[(i * 4) + j].y;

					// UV(0~1) → ローカル座標（中心基準）
					float localX = (u - 0.5f) * wall.obj.Size.x;
					float localY = (v - 0.5f) * wall.obj.Size.y;

					// ローカル → ワールド座標
					if (i == 0) {//一回目　0123->2301
						wallCorners[(j+2)%4].x = wall.obj.Transform.Position.x + localX;
						wallCorners[(j+2)%4].y = wall.obj.Transform.Position.y + localY;
					}
					else {//二回目~　0123->3201
						switch (j)
						{
							case 0:
							wallCorners[2].x = wall.obj.Transform.Position.x + localX;
							wallCorners[2].y = wall.obj.Transform.Position.y + localY;
							break;

							case 1:
							wallCorners[3].x = wall.obj.Transform.Position.x + localX;
							wallCorners[3].y = wall.obj.Transform.Position.y + localY;
							break;

							case 2:
							wallCorners[1].x = wall.obj.Transform.Position.x + localX;
							wallCorners[1].y = wall.obj.Transform.Position.y + localY;
							break;

							case 3:
							wallCorners[0].x = wall.obj.Transform.Position.x + localX;
							wallCorners[0].y = wall.obj.Transform.Position.y + localY;
							break;

						default:
							break;
						}
					}
				}


				for (int j = 0; j < 4; j++)
				{
					shadowVerts[j] = {//0123=左上右上左下右下
						wallCorners[j].x + shadowVec.x * wall.shadowHeight,
						wallCorners[j].y + shadowVec.y * wall.shadowHeight
					};
				}

				// 太陽の方向から影の4頂点を計算
				DynamicShadow shadow;
				shadow.active = false;

				DynamicShadow shadow2;
				shadow2.active = false;

				if (sunVec.x == 0.0f) {
					shadow.active = true;
					if (sunVec.y > 0.0f) {//影真下
						shadow.shadowObj.Vertexs[0] = wallCorners[2];  // 左下（壁）
						shadow.shadowObj.Vertexs[1] = wallCorners[3];  // 右下（壁）
						shadow.shadowObj.Vertexs[2] = shadowVerts[2];  // 左下（影）
						shadow.shadowObj.Vertexs[3] = shadowVerts[3];  // 右下（影）
					}
					else {//影真上
						shadow.shadowObj.Vertexs[0] = shadowVerts[0];  // 左上（影）
						shadow.shadowObj.Vertexs[1] = shadowVerts[1];  // 右上（影）
						shadow.shadowObj.Vertexs[2] = wallCorners[0];  // 左上（壁）
						shadow.shadowObj.Vertexs[3] = wallCorners[1];  // 右上（壁）
					}
				}
				else if (sunVec.y == 0.0f) {
					shadow.active = true;
					if (sunVec.x > 0.0f) {//影真左
						shadow.shadowObj.Vertexs[0] = shadowVerts[0]; //左上（影）
						shadow.shadowObj.Vertexs[1] = wallCorners[0];  // 左上（壁）
						shadow.shadowObj.Vertexs[2] = shadowVerts[2]; // 左下（影）
						shadow.shadowObj.Vertexs[3] = wallCorners[2];  // 左下（壁）
					}
					else {//影真右
						shadow.shadowObj.Vertexs[0] = wallCorners[1];  // 右上（壁）
						shadow.shadowObj.Vertexs[1] = shadowVerts[1];  // 右上（影）
						shadow.shadowObj.Vertexs[2] = wallCorners[3];  // 右下（壁）
						shadow.shadowObj.Vertexs[3] = shadowVerts[3]; // 右下（影）
					}
				}
				else {
					shadow.active = true;
					shadow2.active = true;
					if (sunVec.x > 0.0f && sunVec.y > 0.0f) {//影左下
						shadow.shadowObj.Vertexs[0] = shadowVerts[0];
						shadow.shadowObj.Vertexs[1] = wallCorners[0];
						shadow.shadowObj.Vertexs[2] = shadowVerts[2];
						shadow.shadowObj.Vertexs[3] = wallCorners[2];

						shadow2.shadowObj.Vertexs[0] = wallCorners[2];
						shadow2.shadowObj.Vertexs[1] = wallCorners[3];
						shadow2.shadowObj.Vertexs[2] = shadowVerts[2];
						shadow2.shadowObj.Vertexs[3] = shadowVerts[3];
					}
					else if (sunVec.x > 0.0f && sunVec.y < 0.0f) {//影左上
						shadow.shadowObj.Vertexs[0] = shadowVerts[0];
						shadow.shadowObj.Vertexs[1] = wallCorners[0];
						shadow.shadowObj.Vertexs[2] = shadowVerts[2];
						shadow.shadowObj.Vertexs[3] = wallCorners[2];

						shadow2.shadowObj.Vertexs[0] = shadowVerts[0];
						shadow2.shadowObj.Vertexs[1] = shadowVerts[1];
						shadow2.shadowObj.Vertexs[2] = wallCorners[0];
						shadow2.shadowObj.Vertexs[3] = wallCorners[1];
					}
					else if (sunVec.x < 0.0f && sunVec.y < 0.0f) {//影右上
						shadow.shadowObj.Vertexs[0] = wallCorners[1];
						shadow.shadowObj.Vertexs[1] = shadowVerts[1];
						shadow.shadowObj.Vertexs[2] = wallCorners[3];
						shadow.shadowObj.Vertexs[3] = shadowVerts[3];

						shadow2.shadowObj.Vertexs[0] = shadowVerts[0];
						shadow2.shadowObj.Vertexs[1] = shadowVerts[1];
						shadow2.shadowObj.Vertexs[2] = wallCorners[0];
						shadow2.shadowObj.Vertexs[3] = wallCorners[1];
					}
					else if (sunVec.x < 0.0f && sunVec.y > 0.0f) {//影右下
						shadow.shadowObj.Vertexs[0] = wallCorners[1];
						shadow.shadowObj.Vertexs[1] = shadowVerts[1];
						shadow.shadowObj.Vertexs[2] = wallCorners[3];
						shadow.shadowObj.Vertexs[3] = shadowVerts[3];

						shadow2.shadowObj.Vertexs[0] = wallCorners[2];
						shadow2.shadowObj.Vertexs[1] = wallCorners[3];
						shadow2.shadowObj.Vertexs[2] = shadowVerts[2];
						shadow2.shadowObj.Vertexs[3] = shadowVerts[3];
					}
				}
				
				// 影の中心とサイズを計算（当たり判定用）
				float minX = shadow.shadowObj.Vertexs[0].x;
				float maxX = shadow.shadowObj.Vertexs[0].x;
				float minY = shadow.shadowObj.Vertexs[0].y;
				float maxY = shadow.shadowObj.Vertexs[0].y;

				for (int i = 1; i < 4; i++)
				{
					if (shadow.shadowObj.Vertexs[i].x < minX) minX = shadow.shadowObj.Vertexs[i].x;
					if (shadow.shadowObj.Vertexs[i].x > maxX) maxX = shadow.shadowObj.Vertexs[i].x;
					if (shadow.shadowObj.Vertexs[i].y < minY) minY = shadow.shadowObj.Vertexs[i].y;
					if (shadow.shadowObj.Vertexs[i].y > maxY) maxY = shadow.shadowObj.Vertexs[i].y;
				}

				shadow.shadowObj.Transform.Position.x = (minX + maxX) / 2.0f;
				shadow.shadowObj.Transform.Position.y = (minY + maxY) / 2.0f;
				shadow.shadowObj.Size.x = maxX - minX;
				shadow.shadowObj.Size.y = maxY - minY;
				shadow.shadowObj.Transform.Rotation = 0.0f;
				shadow.shadowObj.Transform.Scale.x = 1.0f;
				shadow.shadowObj.Transform.Scale.y = 1.0f;

				// テクスチャとZ値
				shadow.shadowObj.texNo = g_ShadowTexture;
				shadow.shadowObj.Z_Test = 0.2f;

				// 色（黒色、50%透明）
				shadow.shadowObj.Color.R = 0.0f;
				shadow.shadowObj.Color.G = 0.0f;
				shadow.shadowObj.Color.B = 0.0f;
				shadow.shadowObj.Color.A = 0.5f;

				shadow.shadowObj.flipX = false;
				shadow.shadowObj.flipY = false;

				g_DynamicShadows.push_back(shadow);

				if (shadow2.active == true) {
					// 影の中心とサイズを計算（当たり判定用）
					minX = shadow2.shadowObj.Vertexs[0].x;
					maxX = shadow2.shadowObj.Vertexs[0].x;
					minY = shadow2.shadowObj.Vertexs[0].y;
					maxY = shadow2.shadowObj.Vertexs[0].y;

					for (int i = 1; i < 4; i++)
					{
						if (shadow2.shadowObj.Vertexs[i].x < minX) minX = shadow2.shadowObj.Vertexs[i].x;
						if (shadow2.shadowObj.Vertexs[i].x > maxX) maxX = shadow2.shadowObj.Vertexs[i].x;
						if (shadow2.shadowObj.Vertexs[i].y < minY) minY = shadow2.shadowObj.Vertexs[i].y;
						if (shadow2.shadowObj.Vertexs[i].y > maxY) maxY = shadow2.shadowObj.Vertexs[i].y;
					}

					shadow2.shadowObj.Transform.Position.x = (minX + maxX) / 2.0f;
					shadow2.shadowObj.Transform.Position.y = (minY + maxY) / 2.0f;
					shadow2.shadowObj.Size.x = maxX - minX;
					shadow2.shadowObj.Size.y = maxY - minY;
					shadow2.shadowObj.Transform.Rotation = 0.0f;
					shadow2.shadowObj.Transform.Scale.x = 1.0f;
					shadow2.shadowObj.Transform.Scale.y = 1.0f;

					// テクスチャとZ値
					shadow2.shadowObj.texNo = g_ShadowTexture;
					shadow2.shadowObj.Z_Test = 0.2f;

					// 色（黒色、50%透明）
					shadow2.shadowObj.Color.R = 0.0f;
					shadow2.shadowObj.Color.G = 0.0f;
					shadow2.shadowObj.Color.B = 0.0f;
					shadow2.shadowObj.Color.A = 0.5f;

					shadow2.shadowObj.flipX = false;
					shadow2.shadowObj.flipY = false;

					g_DynamicShadows.push_back(shadow2);
				}
			}
		}
	}
}
