#include "pch.h"
#include "Map.h"
#include <cmath>
#include <iostream>

const float Map::TILE_SIZE = 2.0f;

Map::Map() : m_width(0), m_height(0)
{
}

Map::~Map()
{
}

void Map::LoadDefault()
{
    // 1 = Mur (AABB), 0 = Sol (Marchable), 2 = Spawn
    m_grid = {
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 2, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 2, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
    };

    m_height = static_cast<int>(m_grid.size());
    m_width = static_cast<int>(m_grid[0].size());

    m_aabbs.clear();
    float fullSize = TILE_SIZE * 2.0f;
    float wallHeight = 4.0f;

    for (int z = 0; z < m_height; z++)
    {
        for (int x = 0; x < m_width; x++)
        {
            if (m_grid[z][x] == 1)
            {
                cpu_aabb box;
                box.min = XMFLOAT3(x * fullSize, -wallHeight, z * fullSize);
                box.max = XMFLOAT3((x + 1) * fullSize, wallHeight, (z + 1) * fullSize);

                m_aabbs.push_back(box);
            }
        }
    }
}

bool Map::IsColliding(const Vector3f& worldPos) const
{
    // On ne prépare que X et Z pour le test
    float px = worldPos.x;
    float pz = worldPos.z;

    float fullSize = TILE_SIZE * 2.0f;
    int gx = static_cast<int>(std::floor(px / fullSize));
    int gz = static_cast<int>(std::floor(pz / fullSize));

    if (gz < 0 || gz >= m_height || gx < 0 || gx >= m_width)
        return true;

    for (const cpu_aabb& box : m_aabbs)
    {
        if (px > box.min.x && px < box.max.x &&
            pz > box.min.z && pz < box.max.z)
        {
            return true;
        }
    }

    return false;
}

bool Map::IsWalkable(Vector3f worldPos, Vector3f worldDir) const
{
    float offsetDist = TILE_SIZE;

    Vector3f pointsToTest[4] = {
        { worldPos.x + offsetDist, worldPos.y, worldPos.z },
        { worldPos.x - offsetDist, worldPos.y, worldPos.z },
        { worldPos.x, worldPos.y, worldPos.z + offsetDist },
        { worldPos.x, worldPos.y, worldPos.z - offsetDist }
    };

    for (const auto& testPos : pointsToTest)
    {
        float fullSize = TILE_SIZE * 2.0f;
        int gx = static_cast<int>(std::floor(testPos.x / fullSize));
        int gz = static_cast<int>(std::floor(testPos.z / fullSize));

        if (gz < 0 || gz >= m_height || gx < 0 || gx >= m_width)
            return false;

        XMFLOAT3 p = { testPos.x, testPos.y, testPos.z };
        for (cpu_aabb box : m_aabbs)
        {
            if (box.Contains(p))
            {
                return false;
            }
        }
    }

    return true; // Les 4 points sont dans le vide, on peut marcher
}

Vector3f Map::GetWorldPos(int x, int z) const
{
    float fullSize = TILE_SIZE * 2.0f;
    float halfSize = TILE_SIZE;

    return Vector3f(
        (x * fullSize) + halfSize,
        0.0f,
        (z * fullSize) + halfSize
    );
}

int Map::GetTile(int x, int z) const
{
    if (z < 0 || z >= m_height || x < 0 || x >= m_width) return 1;
    return m_grid[z][x];
}

std::vector<Vector3f> Map::GetSpawnPoints() const
{
    std::vector<Vector3f> spawnPoints;

    for (int z = 0; z < m_height; z++)
    {
        for (int x = 0; x < m_width; x++)
        {
            if (m_grid[z][x] == 2)
            {
                spawnPoints.push_back(GetWorldPos(x, z));
            }
        }
    }

    return spawnPoints;
}

int Map::GetWidth() const { return m_width; }
int Map::GetHeight() const { return m_height; }