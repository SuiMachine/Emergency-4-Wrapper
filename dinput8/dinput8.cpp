#include "dinput8.h"

//Global Variables
int bWidth;
int bHeight;
bool bFullscreen;
bool bSkipIntros;

HMODULE baseModule;

//Detours
DWORD initializeOverrideReturn;
void __declspec(naked) initializeOverride()
{
	_asm
	{
		mov eax, [eax+0x18]
		mov ecx, DS:[0xBD9538]
		mov edx, DS:[0xBD94C8]
		push 0
		push eax
		movzx eax, [bFullscreen]
		mov DS:[0xBD937C],eax
		push    0
		push    eax
		mov     eax, DS:[0xBD9458]
		push    ecx
		mov     ecx, DS:[0xBD93E8]
		push    edx
		push    eax
		push    ecx
		jmp[initializeOverrideReturn]
	}
}

//Dll Main
bool WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			//Get module location and its ini file
			char path[MAX_PATH];
			HMODULE hm = NULL;
			GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)"dinput8.dll", &hm);
			GetModuleFileNameA(hm, path, sizeof(path));
			*strrchr(path, '\\') = '\0';
			strcat_s(path, "\\dinput8.ini");

			//Load info from ini
			bWidth = GetPrivateProfileInt("MAIN", "Width", 0, path);
			bHeight = GetPrivateProfileInt("MAIN", "Height", 0, path);
			if (bWidth == 0 || bHeight == 0)
			{
				bWidth = 1024;
				bHeight = 768;
			}
			bFullscreen = GetPrivateProfileInt("MAIN", "Windowed", 0, path) != 1;
			bSkipIntros = GetPrivateProfileInt("MAIN", "SkipIntros", 0, path) != 0;


			//Get dll from Windows directory
			GetSystemDirectory(path, MAX_PATH);
			strcat_s(path, "\\dinput8.dll");

			//Set pointers
			dinput8.dll = LoadLibraryA(path);
			dinput8.DirectInput8Create = (LPWDirectInput8Create)GetProcAddress(dinput8.dll, "DirectInput8Create");
			dinput8.DllCanUnloadNow = (LPWDllCanUnloadNow)GetProcAddress(dinput8.dll, "DllCanUnloadNow");
			dinput8.DllGetClassObject = (LPWDllGetClassObject)GetProcAddress(dinput8.dll, "DllGetClassObject");
			dinput8.DllRegisterServer = (LPWDllRegisterServer)GetProcAddress(dinput8.dll, "DllRegisterServer");
			dinput8.DllUnregisterServer = (LPWDllUnregisterServer)GetProcAddress(dinput8.dll, "DllUnregisterServer");

			//Get base module
			baseModule = GetModuleHandleA("em4.exe");
			UnprotectModule(baseModule);

			//Modify initialization to enable Windowed mode
			{
				int hookLenght = 0x2B;
				DWORD hookAddress = (DWORD)baseModule + 0x00406DA6;
				initializeOverrideReturn = hookAddress + hookLenght;
				Hook((void*)hookAddress, initializeOverride, hookLenght);
			}

			if (bFullscreen == 0)
			{
				//Disable top-most
				*(byte*)((DWORD)baseModule + 0x33AB0E) = 0;
			}
			
			if (bSkipIntros)
			{
				//Modify jump to skip intros
				*(byte*)((DWORD)baseModule + 0x790D3) = 0xE9;
				*(DWORD*)((DWORD)baseModule + 0x790D4) = 0x000000C1;
			}

			break;
		}
		case DLL_PROCESS_DETACH:
		{
			FreeLibrary(hModule);
			break;
		}
		return true;
	}

	return TRUE;
}

bool Hook(void * toHook, void * ourFunction, int lenght)
{
	if (lenght < 5)
		return false;

	DWORD curProtectionFlag;
	VirtualProtect(toHook, lenght, PAGE_EXECUTE_READWRITE, &curProtectionFlag);
	memset(toHook, 0x90, lenght);
	DWORD relativeAddress = ((DWORD)ourFunction - (DWORD)toHook) - 5;

	*(BYTE*)toHook = 0xE9;
	*(DWORD*)((DWORD)toHook + 1) = relativeAddress;

	DWORD temp;
	VirtualProtect(toHook, lenght, curProtectionFlag, &temp);
	return true;
}

HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter)
{
	HRESULT hr = dinput8.DirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);

	return hr;
}

HRESULT WINAPI DllCanUnloadNow()
{
	return dinput8.DllCanUnloadNow();
}

HRESULT WINAPI DllGetClassObject(REFCLSID riidlsid, REFIID riidltf, LPVOID whatever)
{
	return dinput8.DllGetClassObject(riidlsid, riidltf, whatever);
}

HRESULT WINAPI DllRegisterServer()
{
	return dinput8.DllRegisterServer();
}

HRESULT WINAPI DllUnregisterServer()
{
	return dinput8.DllUnregisterServer();
}