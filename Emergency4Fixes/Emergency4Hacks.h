#pragma once
#include "HookFunctions.h"
#include "VideoHack.h"
#include "VisRenderer.h"
#include "../externals/inireader/IniReader.h"

class Emergency4Hacks
{
public:
	Emergency4Hacks();
	VisRenderer* visRenderer;
	int bWidth = 1024;
	int bHeight = 768;
	bool bFullscreen = true;
private:
	HMODULE baseModule;
	bool bSkipIntros = false;
	bool bCorrectAspectRatioOfCinematics = false;

};

