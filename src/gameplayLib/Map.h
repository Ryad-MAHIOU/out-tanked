#pragma once
#include <vector>
#include "Utils.h"

class Map
{
public:
    static const float TILE_SIZE;

    Map();
    ~Map();

    void LoadDefault();

    bool IsColliding(const Vector3f& worldPos) const;
    bool IsWalkable(Vector3f worldPos, Vector3f worldDir) const;

    Vector3f GetWorldPos(int x, int z) const;
    int GetWidth() const;
    int GetHeight() const;
    int GetTile(int x, int z) const;
    std::vector<Vector3f> GetSpawnPoints() const;

private:
    std::vector<std::vector<int>> m_grid;

    std::vector<cpu_aabb> m_aabbs;

    int m_width;
    int m_height;
};