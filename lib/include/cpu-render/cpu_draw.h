#pragma once

struct cpu_draw
{
public:
	XMFLOAT3* tri;
	cpu_vertex_out* vo;
	cpu_material* pMaterial;
	cpu_tile* pTile;
	byte depth;
};
