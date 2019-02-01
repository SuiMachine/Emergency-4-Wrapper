#include "VisRenderer.h"

VisRenderer::VisRenderer(HMODULE baseModule, HMODULE oVisRendererModule)
{
	this->baseModule = baseModule;
	this->oVisRendererModule = oVisRendererModule;
}

#pragma region DetourPerspectiveToAngles
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

int localIsFullscreen;
void __declspec(naked) detouredVisVideoCLSetMode()
{
	__asm
	{
		mov eax,[localIsFullscreen]
		mov [esp+0x14+4],eax		
		mov eax,[esp+0x4+4]
		mov edx,[esp+0x0C+4]
		ret
	}
}

void VisRenderer::InstallDetourVisVideoCLSetMode(bool isFullscreen)
{
	//VisVideo_cl::SetMode(VisVideo_cl *this, int ResolutionX, int ResolutionY, int bpp, int zBufferBits, int IsFullscreen, int a7, int a8, int refreshRate)
	//Stack looks like this:
	//0x00 - this
	//0x04 - ResolutionX
	//0x08 - ResolutionY
	//0x0C - BPP
	//0x10 - zBufferBits
	//0x14 - Fullscreen
	localIsFullscreen = isFullscreen;
	intptr_t pVisVideoSetMode = 0x0;
	if (GetAddressOFExternFunction(oVisRendererModule, "?SetMode@VisVideo_cl@@QAEJHHJJHHHJ@Z", pVisVideoSetMode))
	{
		HookCallTrampoline(pVisVideoSetMode, &detouredVisVideoCLSetMode, 8);
	}



}

VisRenderer::~VisRenderer()
{
}
