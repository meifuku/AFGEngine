#include "hud.h"
#include "util.h"
#include "window.h"


#include <vector>
#include <glad/glad.h>
#include <iostream>


//VAO data for drawing the HUD.
std::vector<float> hudVertArray;

std::vector<float>& GetHudData()
{
	return hudVertArray;
}

std::vector<Bar> InitBars()
{
	//This should be expanded to InitHUD
	hudVertArray.reserve(64);

	std::vector<Bar> bars;
	for(int i = 0; i < 2; ++i)
	{
		Bar lifebar(200, 10, 20, 240, &hudVertArray, 1, 0, static_cast<e_side>(i));
		bars.push_back(lifebar);
	}
	for(int i = 0; i < 2; ++i)
	{
		Bar lifebar(200, 10, 20, 240, &hudVertArray, 0, 0, static_cast<e_side>(i));
		bars.push_back(lifebar);
	}

	CoordinateHelper ch(1024, 1024, 128, 128, hudVertArray, CHUNK_SIZE); //KO thingy in the middle of the screen.
	ch.shrinkBorders = true;
	PushQuad(internalWidth/2.f-21, internalHeight-42, 42, 42, &hudVertArray, ch, 1, 0, true);

	return bars;
}

Bar::Bar(float _width, float _height, float _offsetX, float _offsetY, std::vector<float> *va, float texAtlasIndexX, float texAtlasIndexY, e_side _side) :
	width(_width), height(_height), offsetX(_offsetX), offsetY(_offsetY), texChunkX(texAtlasIndexX), texChunkY(texAtlasIndexY), side(_side)
{
	vertexArray = va;
	index = va->size();
	if (side == RIGHT)
	{   //Push ccw vertex/texture coordinates of the box
		CoordinateHelper ch(1024, 1024, 64, 64, *vertexArray, CHUNK_SIZE); //sizes hardcoded until I code some texture management.
		ch.shrinkBorders = true;
		PushQuad(internalWidth/2.f+offsetX, offsetY, width, height, vertexArray, ch, texChunkX, texChunkY);
	}
	else //left
	{
		CoordinateHelper ch(1024, 1024, 64, 64, *vertexArray, CHUNK_SIZE);
		ch.shrinkBorders = true;
		PushQuad(internalWidth/2.f-offsetX, offsetY, -width, height, vertexArray, ch, texChunkX, texChunkY);
	}
}

void Bar::Resize(float amountX, float amountY)
{
	if(side == RIGHT)
		ModifyQuad(internalWidth/2.f+offsetX, offsetY, width*amountX, height, vertexArray, index, false);
	else
		ModifyQuad(internalWidth/2.f-offsetX, offsetY, -width*amountX, height, vertexArray, index, false);
		/*
	if(side == RIGHT)
		ModifyQuad(0, 0, width*(amountX)-width, height*(amountY)-height, vertexArray, index, false);
	else
		ModifyQuad(0, 0, -width*(amountX)+width, height*(amountY)-height, vertexArray, index, false);*/
}
