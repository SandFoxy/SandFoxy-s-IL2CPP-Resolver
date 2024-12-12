## SandFoxy's IL2CPP Resolver
A run-time API resolver for IL2CPP Unity.

Based On [sneakyevil/IL2CPP_Resolver](https://github.com/sneakyevil/IL2CPP_Resolver)

### Quick Example
```cpp
#include <IL2CPP_Resolver.hpp>

void DllMain()
{
    IL2CPP::Initialize(); // This needs to be called once!

    Unity::CGameObject* m_Local = Unity::GameObject::Find("LocalPlayer");
    Unity::CComponent* m_LocalData = m_Local->GetComponent("PlayerData");
    m_LocalData->SetMemberValue<bool>("CanFly", true);
}
```

Want help? Fell free to ask issues!