#include "main.h"

#include <glad/glad.h>
#include <SDL.h>

#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>

//#include "audio.h"
#include "camera.h"
#include "chara.h"
#include "hud.h"
#include "raw_input.h"
#include "shader.h"
#include "texture.h"
#include "util.h"
#include "window.h"
#include "vao.h"

#ifdef NETPLAY
#include "netplay.h"
#endif

int inputDelay = 0;

const float charTexCoords[8] = {0,0,1,0,1,1,0,1};

const char *texNames[] ={
	"images/background.png",
	"images/hud.png",
	"images/fontactive.png",
	"images/fontdead.png"
	};

int gameState = GS_MENU;


int main(int argc, char** argv)
{
	#ifdef NETPLAY
	if(argc > 1) //Netplay
	{
		if(!NetplayArgs(argc, argv))
			return 0;
	}
	#endif

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

void PlayLoop()
{
	//texture load
	std::vector<Texture> activeTextures;
	
	activeTextures.reserve(4);
	for(int i = 0; i < 4; ++i)
	{
		Texture texture;

		texture.Load(texNames[i]);
		if(i<2)
			texture.Apply();
		else
			texture.Apply(false, false);
			

		texture.Unload();
		activeTextures.push_back(std::move(texture));
	}
	
	
	//hud load
	std::vector<Bar> barHandler = InitBars();

	int32_t gameTicks = 0;
	Camera view;

/*
	pal = 1;
	Character player(-50, 1, "char/vaki.char");
	pal = 0;
	Character player2(50, -1, "char/vaki.char");

	
	player.SetCameraRef(&view);
	player2.SetCameraRef(&view);

	player.setTarget(&player2);
	player2.setTarget(&player);
*/

	input_deque keyBufDelayed[2] = {input_deque(32, key::buf::TRUE_NEUTRAL), input_deque(32, key::buf::TRUE_NEUTRAL)};
	input_deque keyBuf[2]= {input_deque(32, key::buf::TRUE_NEUTRAL), input_deque(32, key::buf::TRUE_NEUTRAL)};
	/*input_deque dummy(32, key::buf::TRUE_NEUTRAL);
	dummy[3] |= key::buf::LEFT;
	dummy[2] |= key::buf::DOWN;
	dummy[1] |= key::buf::LEFT | key::buf::DOWN;
	dummy[0] |= key::buf::UP;*/

	std::ostringstream timerString;
	timerString.precision(6);
	timerString.setf(std::ios::fixed, std::ios::floatfield);

	/*Song testA("audio/372094.ogg");
	testA.playback.startStream();*/

	float stageVertices[] = {
		-480, 0, 	0, 0,
		480, 0,  	1, 0,
		480, 540,  	1, 1,
		-480, 540,	0, 1
	};

	std::vector<float> textVertData;
	textVertData.resize(24*80);

	Vao VaoTexOnly(Vao::F2F2, GL_DYNAMIC_DRAW);
	int hudId = VaoTexOnly.Prepare(GetHudData().size()*sizeof(float), nullptr);
	int stageId = VaoTexOnly.Prepare(sizeof(stageVertices), stageVertices);
	int textId = VaoTexOnly.Prepare(sizeof(float)*textVertData.size(), nullptr);
	VaoTexOnly.Load();
	
	glClearColor(0, 0, 0.1, 1.f); 
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
		/*
		
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
		*/
		
		
		barHandler[B_P1Life].Resize(0.3, 1);
		barHandler[B_P2Life].Resize(0.9, 1);
		VaoTexOnly.UpdateBuffer(hudId, GetHudData().data(), GetHudData().size()*sizeof(float));
		
		glClear(GL_COLOR_BUFFER_BIT);
		
		//Should calculations be performed earlier? Watchout for this
		//mainWindow->context.SetModelView(view.Calculate(player.getXYCoords(), player2.getXYCoords()));
		mainWindow->context.SetModelView(view.Calculate(Point2d<FixedPoint>(-450,0), Point2d<FixedPoint>(450,0)));

		//Draw stage quad
		glBindTexture(GL_TEXTURE_2D, activeTextures[T_STAGELAYER1].id);
		VaoTexOnly.Draw(stageId, 0, GL_TRIANGLE_FAN);

		/* glTexCoordPointer(2, GL_FLOAT, 0, charTexCoords);
		player2.Draw();
		player.Draw(); */
		
		mainWindow->context.SetModelView();
		glBindTexture(GL_TEXTURE_2D, activeTextures[T_HUD].id);
		VaoTexOnly.Draw(hudId, 0, GL_TRIANGLES);

		timerString.seekp(0);
		timerString << "SFP: " << mainWindow->GetSpf() << " FPS: " << 1/mainWindow->GetSpf();

		glBindTexture(GL_TEXTURE_2D, activeTextures[T_FONT].id);
		int count = DrawText(timerString.str(), textVertData, 2, 12);
		VaoTexOnly.UpdateBuffer(textId, textVertData.data());
		VaoTexOnly.Draw(textId, count);



		mainWindow->SwapBuffers();

		//End drawing.
		 ++gameTicks;

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
