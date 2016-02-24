#pragma once

#include "engine.h"


// ======================= FUNCTIONS ====================== //
/*
INetChannel *IEngine::GetNetChannelInfo()
{
	if (!cachedChan)
		return NULL;
		

	return cachedChan;
}*/
bool IEngine::IsInGame()
{
	if (IsDrawingLoadingImage()) return false; //should be default behaviour anyways.

	return this->IsInGameR();

}