#pragma once

struct cpu_obb
{
public:
	XMFLOAT3 pts[8];

public:
	cpu_obb();

	cpu_obb& operator=(const cpu_aabb& aabb);

	void Zero();
	void XM_CALLCONV Transform(FXMMATRIX m);
};
