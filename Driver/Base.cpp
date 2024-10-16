#include "Base.h"
#include <ntifs.h>

void* __cdecl Base::operator new(size_t size, unsigned tag) noexcept
{
	DbgPrint("[Base]Operator new run.\r\n");
	return ExAllocatePool2(POOL_FLAG_PAGED, size, tag);
}

void __cdecl Base::operator delete(void* ptr) noexcept
{
	DbgPrint("[Base]Operator delete run.\r\n");
	ExFreePool(ptr);
}
