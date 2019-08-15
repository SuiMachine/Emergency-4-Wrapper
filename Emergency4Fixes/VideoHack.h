#pragma once
#include "HookFunctions.h"

class VideoHack
{
public:
	VideoHack(HMODULE baseModule);
	void InstallVideoHack(int* surfaceWidth, int* surfaceHeight);
	~VideoHack();
private:
	HMODULE baseModule;
};

