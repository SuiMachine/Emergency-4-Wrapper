#include "VisRenderer.h"

VisRenderer::VisRenderer(HMODULE baseModule, HMODULE oVisRendererModule)
{
	this->baseModule = baseModule;
	this->oVisRendererModule = oVisRendererModule;
}

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
		HookTrampoline(pPerspectiveToAnglesStart, &detouredPerspectiveToAngles, 5);
	}
}

VisRenderer::~VisRenderer()
{
}
