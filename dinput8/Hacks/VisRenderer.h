#pragma once
#include "../hookFunctions.h"
#include <math.h>

class VisRenderer
{
public:
	VisRenderer(HMODULE baseModule, HMODULE oVisRendererModule);
	void InstallDetourPerspectiveToAngles(int * surfaceWidth, int * surfaceHeight);
	void InstallDetourVisVideoCLSetMode(bool isFullscreen);
	~VisRenderer();
private:
	HMODULE baseModule;
	HMODULE oVisRendererModule;
};

