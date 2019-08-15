#pragma once
#include <Windows.h>

static bool GetAddressOFExternFunction(HMODULE pModule, const char* exportFuncName, intptr_t& FunctionStart)
{
	PIMAGE_NT_HEADERS header = (PIMAGE_NT_HEADERS)((intptr_t)pModule + ((PIMAGE_DOS_HEADER)pModule)->e_lfanew);
	PIMAGE_EXPORT_DIRECTORY exports = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)pModule + header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	BYTE** names = (BYTE * *)((intptr_t)pModule + exports->AddressOfNames);
	for (int i = 0; i < exports->NumberOfNames; i++)
	{
		auto expp = (char*)((intptr_t)pModule + (int)names[i]);
		if (strcmp(expp, exportFuncName) == 0x0)
		{
			auto ordinal = *(short*)((intptr_t)pModule + exports->AddressOfNameOrdinals + i * sizeof(short));

			auto addressOfFunction = *(intptr_t*)((intptr_t)pModule + exports->AddressOfFunctions + ordinal * sizeof(int));
			FunctionStart = ((intptr_t)pModule + addressOfFunction);
			return true;
		}
	}
	return false;
}

static void UnprotectModule(HMODULE p_Module)
{
	//This function was provided by Orfeasz
	PIMAGE_DOS_HEADER s_Header = (PIMAGE_DOS_HEADER)p_Module;
	PIMAGE_NT_HEADERS s_NTHeader = (PIMAGE_NT_HEADERS)((DWORD)p_Module + s_Header->e_lfanew);

	SIZE_T s_ImageSize = s_NTHeader->OptionalHeader.SizeOfImage;

	DWORD s_OldProtect;
	VirtualProtect((LPVOID)p_Module, s_ImageSize, PAGE_EXECUTE_READWRITE, &s_OldProtect);
}

static bool HookInsideFunction(DWORD targetToHook, void* ourFunction, DWORD* returnAddress, int overrideLenght)
{
	if (overrideLenght < 5)
		return false;

	*returnAddress = targetToHook + overrideLenght;

	DWORD curProtectionFlag;
	VirtualProtect((void*)targetToHook, overrideLenght, PAGE_EXECUTE_READWRITE, &curProtectionFlag);
	memset((void*)targetToHook, 0x90, overrideLenght);
	DWORD relativeAddress = ((DWORD)ourFunction - (DWORD)targetToHook) - 5;

	*(BYTE*)targetToHook = 0xE9;
	*(DWORD*)((DWORD)targetToHook + 1) = relativeAddress;

	DWORD temp;
	VirtualProtect((void*)targetToHook, overrideLenght, curProtectionFlag, &temp);
	return true;
}

static bool HookJmpTrampoline(DWORD targetToHook, void* ourFunction, int overrideLenght)
{
	if (overrideLenght < 5)
		return false;

	DWORD curProtectionFlag;
	VirtualProtect((void*)targetToHook, overrideLenght, PAGE_EXECUTE_READWRITE, &curProtectionFlag);
	memset((void*)targetToHook, 0x90, overrideLenght);
	DWORD relativeAddress = ((DWORD)ourFunction - (DWORD)targetToHook) - 5;

	*(BYTE*)targetToHook = 0xE9;
	*(DWORD*)((DWORD)targetToHook + 1) = relativeAddress;

	DWORD temp;
	VirtualProtect((void*)targetToHook, overrideLenght, curProtectionFlag, &temp);
	return true;
}

template<class Out, class In>
Out type_pun(In x)
{
	union {
		In a;
		Out b;
	};
	a = x;
	return b;
};

static bool HookCallTrampoline(DWORD targetToHook, void* ourFunction, int overrideLenght)
{
	if (overrideLenght < 5)
		return false;

	DWORD curProtectionFlag;
	VirtualProtect((void*)targetToHook, overrideLenght, PAGE_EXECUTE_READWRITE, &curProtectionFlag);
	memset((void*)targetToHook, 0x90, overrideLenght);
	DWORD relativeAddress = ((DWORD)ourFunction - (DWORD)targetToHook) - 5;

	*(BYTE*)targetToHook = 0xE8;
	*(DWORD*)((DWORD)targetToHook + 1) = relativeAddress;

	DWORD temp;
	VirtualProtect((void*)targetToHook, overrideLenght, curProtectionFlag, &temp);
	return true;
}

static int StrEndsWith(char* chrArray, int lenght, char character)
{
	int pos = -1;
	for (int i = 0; i < lenght; i++)
	{
		if (chrArray[i] == character)
			pos = i;
	}

	return pos;
}

static void StrToLower(char* chrArray, int Lenght)
{
	for (int i = 0; i < Lenght; i++)
	{
		chrArray[i] = ::tolower(chrArray[i]);
	}
}