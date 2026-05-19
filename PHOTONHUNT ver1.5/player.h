// =========================================================
// player.h プレイヤー制御
// 
// 制作者:		日付：
// =========================================================
#ifndef _PLAYER_H_
#define _PLAYER_H_

// =========================================================
// 構造体宣言
// =========================================================

Microsoft::WRL::ComPtr<ID3D11VertexShader>& GetpVS();
void InitializePlayer(void);
void UpdatePlayer(void);
void DrawPlayer(void);
void FinalizePlayer(void);


#endif