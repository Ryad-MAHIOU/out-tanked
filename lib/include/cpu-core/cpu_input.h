#pragma once

struct cpu_input
{
private:
	enum
	{
		_NONE,
		_DOWN,
		_UP,
		_PUSH,
	};

private:
	byte keys[256];

public:
	static cpu_input& GetInstance();

	void Update();

	void Reset();
	bool IsKey(int key);
	bool IsKeyDown(int key);
	bool IsKeyUp(int key);

	std::vector<int> GetKeys();
	std::vector<int> GetKeysDown();
	std::vector<int> GetKeysUp();

private:
	cpu_input();
	~cpu_input();
};
