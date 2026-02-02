#pragma once

struct cpu_vertex
{
public:
	XMFLOAT3 pos;
	XMFLOAT3 color;
	XMFLOAT3 normal;

public:
	cpu_vertex();

	void Identity();
};
