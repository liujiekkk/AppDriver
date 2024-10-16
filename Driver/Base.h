#pragma once

class Base
{

public:

	// 重定义 new 操作符.
	static void* __cdecl operator new(size_t size, unsigned tag = 'base') noexcept;

	// 重定义 delete 操作符.
	static void __cdecl operator delete(void* ptr) noexcept;

	// 析构函数.
	virtual ~Base() {}
};

