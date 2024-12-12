#pragma once

namespace IL2CPP
{

	/*
	* Easyest way to interact with game own functions.
	* 
	* Exemple:
	*	IL2CPP::Call<Unity::Color>(IL2CPP::Pointer("UnityEngine.Material", "GetColor", 1), *object*)
	*/

	template<typename ret, typename... parameters>
	static ret Call(uintptr_t address, parameters... params)
	{
		typedef ret(*func_t)(parameters...);
		auto func = reinterpret_cast<func_t>(address);

		return func(params...);
	}
	template<typename ret, typename... parameters>
	static ret Call(void* methodPointer, parameters... params)
	{
		typedef ret(*func_t)(parameters...);
		auto func = reinterpret_cast<func_t>(methodPointer);

		return func(params...);
	}

	// We're keeping this shit because the original owner wants to use it :(
	void* ResolveCall(const char* m_Name)
	{
		return reinterpret_cast<void* (IL2CPP_CALLING_CONVENTION)(const char*)>(Functions.m_ResolveFunction)(m_Name);
	}
}