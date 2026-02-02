#pragma once

struct cpu_material
{
public:
	byte lighting;
	CPU_PS_FUNC ps;
	XMFLOAT3 color;
	void* values;

public:
	cpu_material();
};
