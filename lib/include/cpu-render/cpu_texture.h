#pragma once

struct cpu_texture
{
public:
	byte* bgra;
	int width;
	int height;
	int count;
	int size;

public:
	cpu_texture();
	~cpu_texture();

	bool Load(const char* path);
	void Close();
};
