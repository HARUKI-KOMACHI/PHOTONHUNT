#pragma once
#include <vector>
#include <string>



#include <DirectXMath.h>
#include <unordered_set>

class MapGrid
{
public:
    MapGrid() = default;

    // 从文件加载
    bool LoadFromFile(const std::string& filePath);

    // 基本信息
    int GetWidth()  const { return m_width; }
    int GetHeight() const { return m_height; }

    // 通过 (x, y) 获取地块编号
    int GetTile(int x, int y) const;

    // 通过编号，获取所有对应坐标（grid 坐标）
    std::vector<DirectX::XMFLOAT2> GetAllTilesById(int tileId) const;

    // (x, y) + 方块尺寸 → 世界坐标（XZ）
    DirectX::XMFLOAT3 GridToWorld(int x, int y, float tileSize) const;

    // 根据 tileId，从地图生成 Object_3D
    void GenerateObjects(
        const Object_3D* templateObj,     // 模板（必须是指针）
        float tileSize,
        const DirectX::XMFLOAT3& originWorldPos = { 0,0,0 },
        int originGridX = 0,
        int originGridY = 0
    );

    std::vector<Object_3D>& Get_obj_maps(void);

    void ObjectsWithTileTexture(int ID, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& text);
    DirectX::XMFLOAT4 GetTileColor(int tileId) const;

    // 移动判定（基于地块坐标）
    bool CanMoveTo(int gridX, int gridY) const;

    // 黑名单管理
    void AddBlockedTileId(int id);
    void RemoveBlockedTileId(int id);

private:
    int m_width = 0;
    int m_height = 0;

    // 一维地块数据
    std::vector<int> m_tiles;
    std::vector<Object_3D> obj_maps;
    // 颜色表：tileId -> RGBA(0~1)
    std::unordered_map<int, DirectX::XMFLOAT4> m_tileColors;

    // 地块不可通行 ID 黑名单
    std::unordered_set<int> m_blockedIds{ -1 }; // 默认包含 -1
};

void InitializeGame_Map(void);
void UpdateGame_Map(void);
void DrawGame_Map(void);
void FinalizeGame_Map(void);


std::vector<std::string> GetMapFileList(const std::string& folderPath);

MapGrid& GetMap(void);