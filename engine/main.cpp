#include "main.h"

#include <glad/glad.h>
#include <SDL.h>
#include <glm/ext/matrix_transform.hpp>

#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <vector>

//#include "audio.h"
#include "camera.h"
#include "chara.h"
#include "hud.h"
#include "raw_input.h"
#include <shader.h>
#include <texture.h>
#include "util.h"
#include "window.h"
#include <vao.h>

int inputDelay = 0;

const char *texNames[] ={
	"data/images/background.png",
	"data/images/hud.png",
	"data/images/font.png",
	"data/images/vaki.png",
	};

int gameState = GS_MENU;


int main(int argc, char** argv)
{
	mainWindow = new Window();

	//reads configurable keys
	std::ifstream keyfile("keyconf.bin", std::ifstream::in | std::ifstream::binary);
	if(keyfile.is_open())
	{
		keyfile.read((char*)modifiableSCKeys, sizeof(int)*buttonsN*2);
		keyfile.close();
	}
	keyfile.open("joyconf.bin", std::ifstream::in | std::ifstream::binary);
	if(keyfile.is_open())
	{
		keyfile.read((char*)modifiableJoyKeys, sizeof(int)*4);
		keyfile.close();
	}

	while(!mainWindow->wantsToClose)
	{
		switch (gameState)
		{
		case GS_MENU:
			//break; Not implemented so have a fallthru.
		case GS_CHARSELECT:
			//break;
		case GS_PLAY:
			PlayLoop();
			break;
		}
	}

	return 0;
}

int LoadPaletteTEMP()
{
	std::ifstream pltefile("data/palettes/play2.act", std::ifstream::in | std::ifstream::binary);
	uint8_t palette[256*3*2];

	pltefile.read((char*)palette, 256*3);
	pltefile.close();
	pltefile.open("data/palettes/akicolor.act", std::ifstream::in | std::ifstream::binary);
	pltefile.read((char*)(palette+256*3), 256*3);

	GLuint paletteGlId;

	glGenTextures(1, &paletteGlId);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, paletteGlId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, palette);
	glActiveTexture(GL_TEXTURE0);

	return paletteGlId;
}

void PlayLoop()
{
	//texture load
	std::vector<Texture> activeTextures;
	activeTextures.reserve(4);
	for(int i = 0; i < 4; ++i)
	{
		Texture texture;

		if(i==3)
			texture.Load(texNames[i], "", true);
		else
			texture.Load(texNames[i]);
		
		if(i<2)
			texture.Apply(false,true);
		else
			texture.Apply();

		texture.Unload();
		activeTextures.push_back(std::move(texture));
	}
	LoadPaletteTEMP();
	
	//hud load
	std::vector<Bar> barHandler = InitBars();

	std::ostringstream timerString;
	timerString.precision(6);
	timerString.setf(std::ios::fixed, std::ios::floatfield);

	int hudId, stageId, textId;
	std::vector<float> textVertData;
	textVertData.resize(24*80);
	Vao VaoTexOnly(Vao::F2F2, GL_DYNAMIC_DRAW);
	{
		float stageVertices[] = {
			-480, 0, 	0, 0,
			480, 0,  	1, 0,
			480, 540,  	1, 1,
			-480, 540,	0, 1
		};
		
		hudId = VaoTexOnly.Prepare(GetHudData().size()*sizeof(float), nullptr);
		stageId = VaoTexOnly.Prepare(sizeof(stageVertices), stageVertices);
		textId = VaoTexOnly.Prepare(sizeof(float)*textVertData.size(), nullptr);
		VaoTexOnly.Load();
	}

	Vao VaoChar(Vao::F2F2I1, GL_STATIC_DRAW);
	{
		int nSprites, nChunks;
		std::ifstream vertexFile("data/images/vaki.vt8", std::ios_base::binary);

		vertexFile.read((char *)&nSprites, sizeof(int));
		auto chunksPerSprite = new uint16_t[nSprites];
		vertexFile.read((char *)chunksPerSprite, sizeof(uint16_t)*nSprites);

		vertexFile.read((char *)&nChunks, sizeof(int));
		auto vertexData = new VertexData8[nChunks*6]; 
		vertexFile.read((char *)vertexData, nChunks*6*sizeof(VertexData8));

		unsigned int chunkCount = 0;
		for(int i = 0; i < nSprites; i++)
		{
			uint8_t strLen;
			vertexFile.read((char*)&strLen, 1);

			//Ignore namemap's name and index.
			vertexFile.ignore(strLen);
			vertexFile.ignore(sizeof(uint16_t));

			VaoChar.Prepare(sizeof(VertexData8)*6*chunksPerSprite[i], &vertexData[6*chunkCount]);
			chunkCount += chunksPerSprite[i];
		}
		std::cout << "Chunks read: "<<chunkCount<<"/"<<nChunks<<"\n";
		vertexFile.close();

		VaoChar.Load();
		delete[] vertexData;
		delete[] chunksPerSprite;
	}
	Camera view;

	Character player(-50, 1, "data/char/vaki.char");
	Character player2(50, -1, "data/char/vaki.char");

	player.SetCameraRef(&view);
	player2.SetCameraRef(&view);

	player.setTarget(&player2);
	player2.setTarget(&player);

	input_deque keyBufDelayed[2] = {input_deque(32, key::buf::TRUE_NEUTRAL), input_deque(32, key::buf::TRUE_NEUTRAL)};
	input_deque keyBuf[2]= {input_deque(32, key::buf::TRUE_NEUTRAL), input_deque(32, key::buf::TRUE_NEUTRAL)};


	VaoTexOnly.Bind();
	glClearColor(1, 1, 1, 1.f); 

	int32_t gameTicks = 0;
	bool gameOver = false;
	while(!gameOver && !mainWindow->wantsToClose)
	{
		if(int err = glGetError())
		{
			std::cerr << "GL Error: 0x" << std::hex << err << "\n";
		}
		
		EventLoop();
		
		// TODO
		//if(glfwJoystickPresent(GLFW_JOYSTICK_1))
		//	GameLoopJoy();
		
		for(int i = 0; i < 2; ++i)
		{
			keyBuf[i].pop_back();
			keyBuf[i].push_front(keySend[i]);
			keyBufDelayed[i].pop_back();
			keyBufDelayed[i].push_front(keyBuf[i][inputDelay]);
		}
		
		player.HitCollision();
		player2.HitCollision();

		player.Input(&keyBufDelayed[0]);
		player2.Input(&keyBufDelayed[1]);

		player.Update();
		player2.Update();

		Character::Collision(&player, &player2);

		barHandler[B_P1Life].Resize(player.getHealthRatio(), 1);
		barHandler[B_P2Life].Resize(player2.getHealthRatio(), 1);
		VaoTexOnly.UpdateBuffer(hudId, GetHudData().data(), GetHudData().size()*sizeof(float));
		

		//Start rendering
		glClear(GL_COLOR_BUFFER_BIT);
		
		//Should calculations be performed earlier? Watchout for this (Why?)
		glm::mat4 viewMatrix = view.Calculate(player.getXYCoords(), player2.getXYCoords());
		mainWindow->context.SetModelView(viewMatrix);

		//Draw stage quad
		glBindTexture(GL_TEXTURE_2D, activeTextures[T_STAGELAYER1].id);
		VaoTexOnly.Draw(stageId, 0, GL_TRIANGLE_FAN);

		//Draw characters
		mainWindow->context.SetShader(RenderContext::PALETTE);
		glBindTexture(GL_TEXTURE_2D, activeTextures[T_CHAR].id);
		VaoChar.Bind();
		mainWindow->context.SetPaletteSlot(1);
		mainWindow->context.SetModelView(viewMatrix*player2.GetSpriteTransform());
		VaoChar.Draw(player2.GetSpriteIndex());

		mainWindow->context.SetPaletteSlot(0);
		mainWindow->context.SetModelView(viewMatrix*player.GetSpriteTransform());
		VaoChar.Draw(player.GetSpriteIndex());

		//Draw HUD
		VaoTexOnly.Bind();
		mainWindow->context.SetShader(RenderContext::DEFAULT);
		mainWindow->context.SetModelView();
		glBindTexture(GL_TEXTURE_2D, activeTextures[T_HUD].id);
		VaoTexOnly.Draw(hudId);

		timerString.seekp(0);
		timerString << "SFP: " << mainWindow->GetSpf() << " FPS: " << 1/mainWindow->GetSpf();

		glBindTexture(GL_TEXTURE_2D, activeTextures[T_FONT].id);
		int count = DrawText(timerString.str(), textVertData, 2, 10);
		VaoTexOnly.UpdateBuffer(textId, textVertData.data());
		VaoTexOnly.Draw(textId, count);

		//End drawing.
		++gameTicks;
		mainWindow->SwapBuffers();
		mainWindow->SleepUntilNextFrame();
	}
}

void EventLoop()
{
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			case SDL_QUIT:
				mainWindow->wantsToClose = true;
				return;
			case SDL_WINDOWEVENT:
				switch(event.window.event)
				{
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						mainWindow->context.UpdateViewport(event.window.data1, event.window.data2);
						break;
				}
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				GameLoopKeyHandle(event.key);
				break;
		}
	}
}
