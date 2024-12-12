# SandFoxy's IL2CPP Resolver
A run-time API resolver for IL2CPP Unity.

Based On [sneakyevil/IL2CPP_Resolver](https://github.com/sneakyevil/IL2CPP_Resolver)


### This fork should not be detected by EAC, just change the WinAPI functions and you're done!
### All strings are protected by default.

### Quick Example
```cpp
#include <IL2CPP_Resolver.hpp>

DWORD WINAPI MainThread(LPVOID lpReserved)
{
	CreateConsole();

	IL2CPP::Initialize(); //Call Once

	Unity::CGameObject* localplayer = Unity::GameObject::Find("Localplayer");
	if (!localplayer)
		return FALSE;

	Unity::CComponent* baseplayer = localplayer->GetComponent("BasePlayer");
	if (!baseplayer)
		return FALSE;

	Unity::System_String* displayName = baseplayer->GetMemberValue<Unity::System_String*>("displayName");
	std::cout << displayName->ToString() << std::endl;

	FreeConsole();
	return TRUE;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);
		CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
		break;
	}
	return TRUE;
}
```

Want help? Fell free to ask in issues!