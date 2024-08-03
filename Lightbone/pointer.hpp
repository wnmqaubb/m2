#pragma once
#include <stdint.h>
#include <windows.h>

using byte_ptr = uint8_t*;
using byte = uint8_t;

template <typename T>
void* get_ptr(T function)
{
	return *(void**)&function;
}

template <typename T, typename T2>
T get_lambda_ptr(T2 ptr)
{
	return ptr;
}

class ptr
{
public:
	inline ptr()
	{
		value = NULL;
	}
	inline ptr(const void* arg) : ptr()
	{
		value = (uintptr_t)arg;
	}
	inline ptr(unsigned int arg) : ptr()
	{
		value = (uintptr_t)arg;
	}
	inline ptr(void* arg) : ptr()
	{
		value = (uintptr_t)arg;
	}
	operator void*() { return (void*)value; }
	operator char*() { return (char*)value; }
	operator unsigned int() { return (unsigned int)value &0xFFFFFFFF; }
	operator HMODULE() { return (HMODULE)value; }

	inline ptr operator *() { return unref(); }
	inline ptr operator +(int imm) { return value + imm; }
	inline ptr operator [](int index)
	{
		return ((uintptr_t*)value)[index];
	}
	inline bool empty()
	{
		return value == 0;
	}
	inline ptr unref()
	{
		return *(void**)value;
	}
	
	template<typename R, typename... Args>
	R std_call(Args...args)
	{
		typedef R(__stdcall *FunctionDefine)(Args...);
		FunctionDefine function = (FunctionDefine)value;
		return function(args...);
	}

	template<typename R, typename... Args>
	R cdecl_call(Args...args)
	{
		typedef R(__cdecl *FunctionDefine)(Args...);
		FunctionDefine function = (FunctionDefine)value;
		return function(args...);
	}

	template<typename R, typename... Args>
	R this_call(void* this_, Args...args)
	{
		typedef R(__thiscall *FunctionDefine)(void* this_, Args...);
		FunctionDefine function = (FunctionDefine)value;
		return function(this_, args...);
	}

	template<typename R, typename... Args>
	R this_call_vtable(int index, Args...args)
	{
		typedef R(__thiscall *FunctionDefine)(uintptr_t this_,Args...);
		FunctionDefine function = (FunctionDefine)(*(uintptr_t**)value)[index];
		return function(value, args...);
	}


	uintptr_t value;
};

