#pragma once


// Default Headers
#include <cstdint>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include <unordered_map>
#include <Windows.h>

#include "skCrypter.h"

// Application Defines
#ifndef UNITY_VERSION_2022_3_8F1
	// If Unity version is equal or greater than 2022.3.8f1 uncomment this define.
	// #define UNITY_VERSION_2022_3_8F1
#endif

// IL2CPP Defines

// Disable Asserts
//#define IL2CPP_ASSERT(x) assert(0)
#ifndef IL2CPP_ASSERT
	#include <cassert>
	#define IL2CPP_ASSERT(x) assert(x)
#endif

#ifndef _DEBUG 
	// If you wanna forward to some string encryption just define before including this file.
	#define IL2CPP_RStr(x) skCrypt(x)
#else
	#define IL2CPP_RStr(x) x
#endif

#ifndef IL2CPP_MAIN_MODULE
	// If the game for some reason uses diff module name just define own one before including this file.
	#define IL2CPP_MAIN_MODULE IL2CPP_RStr(L"GameAssembly.dll")
#endif

#include "Defines.hpp"

// IL2CPP Headers
#include "Data.hpp"

// Unity Headers
#include "Unity/Obfuscators.hpp"
#include "Unity/Defines.hpp"
#include "Unity/Structures/il2cpp.hpp"
#include "Unity/Structures/il2cppArray.hpp"
#include "Unity/Structures/il2cppDictionary.hpp"
#include "Unity/Structures/Engine.hpp"
#include "Unity/Structures/System_String.hpp"

// IL2CPP Utils
#include "Utils/Hash.hpp"
#include "Utils/VTable.hpp"

// IL2CPP API Headers
#include "API/Domain.hpp"
#include "API/Class.hpp"
#include "API/Functions.hpp"
#include "API/String.hpp"
#include "API/Thread.hpp"

// IL2CPP Headers before Unity API
#include "SystemTypeCache.hpp"

// Unity Class APIs - So they're accessible everywhere
namespace Unity
{
	class CCamera;
	class CComponent;
	class CGameObject;
	class CLayerMask;
	class CObject;
	class CRigidbody;
	class CTransform;
}

// Unity API
#include "Unity/API/Object.hpp"
#include "Unity/API/GameObject.hpp"
#include "Unity/API/Camera.hpp"
#include "Unity/API/Component.hpp"
#include "Unity/API/LayerMask.hpp"
#include "Unity/API/Rigidbody.hpp"
#include "Unity/API/Transform.hpp"

// IL2CPP Headers after Unity API
#include "Utils/Helper.hpp"
#include "API/Callback.hpp"
#include <TlHelp32.h>

namespace IL2CPP
{
	namespace Utils {
		bool isEqual(char* names, const wchar_t* API)
		{
			std::wstring wc(API);
			std::string tmp(names);
			std::wstring wc2(tmp.begin(), tmp.end());

			return wc.compare(wc2) == 0;
		}

		/*
		* Our own WinAPI functions for bypassing the simplest anti-cheats
		*/
		MODULEENTRY32W GetModule(const wchar_t* modName)
		{
			uintptr_t modBaseAddr = 0;
			HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetCurrentProcessId());
			if (hSnap != INVALID_HANDLE_VALUE)
			{
				MODULEENTRY32W modEntry;
				modEntry.dwSize = sizeof(modEntry);
				if (Module32FirstW(hSnap, &modEntry))
				{
					do
					{
						if (!_wcsicmp(modEntry.szModule, modName))
						{
							return modEntry;
						}
					} while (Module32NextW(hSnap, &modEntry));
				}
			}
			CloseHandle(hSnap);
			return MODULEENTRY32W();
		}

		void* GetProcAddress(HMODULE hModule, std::string str)
		{
			std::wstring widestr = std::wstring(str.begin(), str.end());
			const wchar_t* wAPIName = widestr.c_str();

			unsigned char* lpBase = reinterpret_cast<unsigned char*>(hModule);
			IMAGE_DOS_HEADER* idhDosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(lpBase);
			if (idhDosHeader->e_magic == 0x5A4D)
			{
				IMAGE_NT_HEADERS64* inhNtHeader = reinterpret_cast<IMAGE_NT_HEADERS64*>(lpBase + idhDosHeader->e_lfanew);
				if (inhNtHeader->Signature == 0x4550)
				{
					IMAGE_EXPORT_DIRECTORY* iedExportDirectory = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(lpBase + inhNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
					for (register unsigned int uiIter = 0; uiIter < iedExportDirectory->NumberOfNames; ++uiIter)
					{
						char* szNames = reinterpret_cast<char*>(lpBase + reinterpret_cast<unsigned long*>(lpBase + iedExportDirectory->AddressOfNames)[uiIter]);
						if (IL2CPP::Utils::isEqual(szNames, wAPIName))
						{
							unsigned short usOrdinal = reinterpret_cast<unsigned short*>(lpBase + iedExportDirectory->AddressOfNameOrdinals)[uiIter];
							return reinterpret_cast<void*>(lpBase + reinterpret_cast<unsigned long*>(lpBase + iedExportDirectory->AddressOfFunctions)[usOrdinal]);
						}
					}
				}
			}
			return 0;
		}
	}

	namespace UnityAPI
	{
		enum m_eExportObfuscationType
		{
			None = 0,
			ROT = 1,
			MAX = 2,
		};
		m_eExportObfuscationType m_ExportObfuscation = m_eExportObfuscationType::None;

		int m_ROTObfuscationValue = -1;
		void* ResolveExport(const char* m_Name)
		{
			switch (m_ExportObfuscation)
			{
				case m_eExportObfuscationType::ROT:
				{
					if (m_ROTObfuscationValue == -1) // Bruteforce
					{
						for (int i = 1; 26 > i; ++i)
						{
							void* m_Return = IL2CPP::Utils::GetProcAddress(Globals.m_GameAssembly, &Unity::Obfuscators::ROT_String(m_Name, i)[0]);
							if (m_Return)
							{
								m_ROTObfuscationValue = i;
								return m_Return;
							}
						}

						return nullptr;
					}

					return IL2CPP::Utils::GetProcAddress(Globals.m_GameAssembly, &Unity::Obfuscators::ROT_String(m_Name, m_ROTObfuscationValue)[0]);
				}
				default: return IL2CPP::Utils::GetProcAddress(Globals.m_GameAssembly, m_Name);
			}

			return nullptr;
		}

		bool ResolveExport_Boolean(void** m_Address, const char* m_Name)
		{
			*m_Address = ResolveExport(m_Name);
			IL2CPP_ASSERT(*m_Address != nullptr && "Couldn't resolve export!");
			return (*m_Address);
		}

		bool Initialize()
		{
			bool m_InitExportResolved = false;
			for (int i = 0; m_eExportObfuscationType::MAX > i; ++i)
			{
				m_ExportObfuscation = static_cast<m_eExportObfuscationType>(i);
				if (ResolveExport(IL2CPP_INIT_EXPORT))
				{
					m_InitExportResolved = true;
					break;
				}
			}

			IL2CPP_ASSERT(m_InitExportResolved && "Couldn't resolve il2cpp_init!");
			if (!m_InitExportResolved) 
				return false;

			//Perfect decryption!
			std::unordered_map<std::string, void**> m_ExportMap =
			{
				{ IL2CPP_CLASS_FROM_NAME_EXPORT.decrypt(),					&Functions.m_ClassFromName},
				{ IL2CPP_CLASS_GET_FIELDS.decrypt(),							&Functions.m_ClassGetFields },
				{ IL2CPP_CLASS_GET_FIELD_FROM_NAME_EXPORT.decrypt(),			&Functions.m_ClassGetFieldFromName },
				{ IL2CPP_CLASS_GET_METHODS.decrypt(),							&Functions.m_ClassGetMethods },
				{ IL2CPP_CLASS_GET_METHOD_FROM_NAME_EXPORT.decrypt(),			&Functions.m_ClassGetMethodFromName },
				{ IL2CPP_CLASS_GET_PROPERTY_FROM_NAME_EXPORT.decrypt(),		&Functions.m_ClassGetPropertyFromName },
				{ IL2CPP_CLASS_GET_TYPE_EXPORT.decrypt(),						&Functions.m_ClassGetType },
				{ IL2CPP_DOMAIN_GET_EXPORT.decrypt(),							&Functions.m_DomainGet },
				{ IL2CPP_DOMAIN_GET_ASSEMBLIES_EXPORT.decrypt(),				&Functions.m_DomainGetAssemblies },
				{ IL2CPP_FREE_EXPORT.decrypt(),								&Functions.m_Free },
				{ IL2CPP_IMAGE_GET_CLASS_EXPORT.decrypt(),					&Functions.m_ImageGetClass },
				{ IL2CPP_IMAGE_GET_CLASS_COUNT_EXPORT.decrypt(),				&Functions.m_ImageGetClassCount },
				{ IL2CPP_RESOLVE_FUNC_EXPORT.decrypt(),						&Functions.m_ResolveFunction },
				{ IL2CPP_STRING_NEW_EXPORT.decrypt(),							&Functions.m_StringNew },
				{ IL2CPP_THREAD_ATTACH_EXPORT.decrypt(),						&Functions.m_ThreadAttach },
				{ IL2CPP_THREAD_DETACH_EXPORT.decrypt(),						&Functions.m_ThreadDetach },
				{ IL2CPP_TYPE_GET_OBJECT_EXPORT.decrypt(),					&Functions.m_TypeGetObject },
				{ IL2CPP_OBJECT_NEW.decrypt(),								&Functions.m_pObjectNew },
				{ IL2CPP_METHOD_GET_PARAM_NAME.decrypt(),						&Functions.m_MethodGetParamName },
				{ IL2CPP_METHOD_GET_PARAM.decrypt(),							&Functions.m_MethodGetParam },
				{ IL2CPP_CLASS_FROM_IL2CPP_TYPE.decrypt(),					&Functions.m_ClassFromIl2cppType },
				{ IL2CPP_FIELD_STATIC_GET_VALUE.decrypt(),					&Functions.m_FieldStaticGetValue },
				{ IL2CPP_FIELD_STATIC_SET_VALUE.decrypt(),					&Functions.m_FieldStaticSetValue },
			};

			for (auto& m_ExportPair : m_ExportMap)
			{
				if (!ResolveExport_Boolean(m_ExportPair.second, &m_ExportPair.first[0]))
					return false;
			}

			// Unity APIs
			Unity::Camera::Initialize();
			Unity::Component::Initialize();
			Unity::GameObject::Initialize();
			Unity::LayerMask::Initialize();
			Unity::Object::Initialize();
			Unity::RigidBody::Initialize();
			Unity::Transform::Initialize();

			// Caches
			IL2CPP::SystemTypeCache::Initializer::PreCache();

			return true;
		}
	}

	/*
	* Gets a pointer to a method (dynamic)
	*
	** For example, let's try to find the address “WorldToViewportPoint”
	*
	* UnityEngine.Camera
	* public Vector3 WorldToViewportPoint(Vector3 position)
	* {
	* return this.WorldToViewportPoint(position, Camera.MonoOrStereoscopicEye.Mono)
	* }
	*
	*
	* Usage:
	*	namespace: “Unity.Camera”
	*	method: “WorldToViewportPoint”
	*	args: 1
	* 
	*/
	void* Pointer(const char* namespace_, const char* method, int args = -1) {
		Unity::il2cppClass* m_pCharacterManager = IL2CPP::Class::Find(namespace_);
		void* Pointer = IL2CPP::Class::Utils::GetMethodPointer(m_pCharacterManager, method, args);

#ifdef _DEBUG
		if (!Pointer) {
			DebugBreak();
		}
#endif // _DEBUG

		return Pointer;
	}


	/*
	* You will get the RVA offset of the method.
	*
	* For example, let's try to find the address “WorldToViewportPoint”.
	*
	* UnityEngine.Camera
	* public Vector3 WorldToViewportPoint(Vector3 position)
	* {
	* return this.WorldToViewportPoint(position, Camera.MonoOrStereoscopicEye.Mono)
	* }
	*
	*
	* Usage:
	*	namespace: “Unity.Camera”
	*	method: “WorldToViewportPoint”
	*	args: 1
	*/
	uintptr_t Method(const char* namespace_, const char* method, int args = -1) {
		Unity::il2cppClass* m_pCharacterManager = IL2CPP::Class::Find(namespace_);
		void* Pointer = IL2CPP::Class::Utils::GetMethodPointer(m_pCharacterManager, method, args);

#ifdef _DEBUG
		if (!Pointer) {
			DebugBreak();
		}
#endif // _DEBUG

		return ((uintptr_t)Pointer) - (uintptr_t)Globals.m_GameAssembly;
	}

	/* 
	*	You need to call this, before using any IL2CPP/Unity Functions!
	*	Args:
	*		m_WaitForModule - Will wait for main module if you're loading your dll earlier than the main module.
	*		m_MaxSecondsWait - Max seconds it will wait for main module to load otherwise will return false to prevent infinite loop.
	*/
	bool Initialize()
	{
		Globals.m_GameAssembly = IL2CPP::Utils::GetModule(IL2CPP_MAIN_MODULE).hModule;

		if (!UnityAPI::Initialize())
			return false;

		return true;
	}
}
