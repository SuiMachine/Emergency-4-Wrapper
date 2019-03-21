#include "VisRenderer.h"

VisRenderer::VisRenderer(HMODULE baseModule, HMODULE oVisRendererModule)
{
	this->baseModule = baseModule;
	this->oVisRendererModule = oVisRendererModule;
}

#pragma region VisRendererPerspectiveToAngles
float pAspectRatio = 1.77777f;
void __stdcall detouredPerspectiveToAngles(float ConfigsFOV, float * FOV_Horizontal, float * FOV_Vertical, float AspectRatio)
{
	//ignoring aspect ratio provided by the game and using our own
	*FOV_Horizontal = atan2f(160.0f / ConfigsFOV, 1.0) * 114.59155f;
	*FOV_Vertical = atan2(160.0f / (ConfigsFOV * pAspectRatio), 1.0f) * 114.59155f;
}

void VisRenderer::InstallDetourPerspectiveToAngles(int * surfaceWidth, int * surfaceHeight)
{
	pAspectRatio = *surfaceWidth * 1.0f / *surfaceHeight;
	intptr_t pPerspectiveToAnglesStart = 0x0;
	if (GetAddressOFExternFunction(oVisRendererModule, "?PerspectiveToAngles@VisRenderer_cl@@QBEXMAAM0M@Z", pPerspectiveToAnglesStart))
	{
		HookJmpTrampoline(pPerspectiveToAnglesStart, &detouredPerspectiveToAngles, 5);
	}
}
#pragma endregion
float pAnisotropy = 1.0f;
typedef void (__thiscall *setAnisotropyInsideClass)(float * th, float a2, char a3);
void __stdcall detouredVisRendererSetMaxAnisotropy(float AnisotropicAmount)
{
	setAnisotropyInsideClass f = (setAnisotropyInsideClass)0x100EE080;
	f((float*)0x10144E98, pAnisotropy, 1);
}

void VisRenderer::InstallDetourVisRendererSetMaxAnistropy(float Anisotropy)
{
	pAnisotropy = Anisotropy;
	intptr_t pVisRendererSetMaxAnisotropy = 0x0;
	if (GetAddressOFExternFunction(oVisRendererModule, "?SetMaxAnisotropy@VisRenderer_cl@@QAEXM@Z", pVisRendererSetMaxAnisotropy))
	{
		HookJmpTrampoline(pVisRendererSetMaxAnisotropy, &detouredVisRendererSetMaxAnisotropy, 6);
	}
}

#pragma region  VisVideoClSetMode
int localWidth;
int localHeight;
int localIsFullscreen;
void __declspec(naked) detouredVisVideoCLSetMode()
{
	//Since this code is called with code assembly instruction, on top of a stack we get additional 4 byte of return address
	//so every instruction accessing the stack inderectly has to be offset by addtional 4 bytes, hence +4
	//Stack looks like this:
	//0x00 - this
	//0x04 - ResolutionX
	//0x08 - ResolutionY
	//0x0C - BPP
	//0x10 - zBufferBits
	//0x14 - Fullscreen
	//0x18 - Unknown
	//0x1C - Unknown
	//0x20 - Refresh Rate
	__asm
	{
		mov eax, [localWidth]
		mov [esp+0x04+4], eax
		mov eax, [localHeight]
		mov [esp+0x08+4], eax
		mov eax,[localIsFullscreen]
		mov [esp+0x14+4],eax		
		mov eax,[esp+0x4+4]
		mov edx,[esp+0x0C+4]
		ret
	}
}

void VisRenderer::InstallDetourVisVideoCLSetMode(int Width, int Height, bool isFullscreen)
{
	localWidth = Width;
	localHeight = Height;
	localIsFullscreen = isFullscreen;
	intptr_t pVisVideoSetMode = 0x0;
	if (GetAddressOFExternFunction(oVisRendererModule, "?SetMode@VisVideo_cl@@QAEJHHJJHHHJ@Z", pVisVideoSetMode))
	{
		HookCallTrampoline(pVisVideoSetMode, &detouredVisVideoCLSetMode, 8);
	}
}
#pragma endregion



VisRenderer::~VisRenderer()
{
}
