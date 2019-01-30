#pragma once
#include "../hookFunctions.h"

class VideoHack
{
public:
	VideoHack(HMODULE baseModule);
	void InstallVideoHack(int * surfaceWidth, int * surfaceHeight);
	~VideoHack();
private:
	HMODULE baseModule;
};

