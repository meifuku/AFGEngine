#include "window.h"
#include <shader.h>

#include <SDL.h>

#include <assert.h>
#include <iostream>
#include <chrono>
#include <thread>

Window *mainWindow = nullptr;

Window::Window() :
wantsToClose(false),
fullscreen(false),
vsync(false),
busyWait(false),
window(nullptr),
frameRateChoice(0),
targetSpf(0.01666),
realSpf(0),
startClock(std::chrono::high_resolution_clock::now())
{
	if(SDL_Init(SDL_INIT_VIDEO))
	{
		std::cerr << SDL_GetError();
		throw std::runtime_error("Couldn't init SDL.");
	}

	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	

	assert(!fullscreen);
	window = SDL_CreateWindow(
		"Another Fighting Game Engine",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, internalWidth*2, internalHeight*2,
		SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);

	if(!window)
	{
		std::cerr << SDL_GetError();
		SDL_Quit();
		throw std::runtime_error("Couldn't create window.");
	}

	context.SetupGl(window);

	if(vsync)
		SDL_GL_SetSwapInterval(1);
}

Window::~Window()
{
	SDL_DestroyWindow( window );
	SDL_Quit();
}

void Window::SleepUntilNextFrame()
{
	if(!vsync)
	{
		std::chrono::duration<double> targetDur(targetSpf);
		std::chrono::duration<double> dur; 
		if(busyWait) //Sleepless wait.
		{
			while( (dur = std::chrono::high_resolution_clock::now() - startClock) <= targetDur)
			{
				
			}
		}
		else
		{
			auto now = std::chrono::high_resolution_clock::now();
			if((dur = now - startClock) < targetDur)
			{
				std::this_thread::sleep_for((targetDur-dur)*0.9);
			}
			while( (dur = std::chrono::high_resolution_clock::now() - startClock) <= targetDur); 
		}
		realSpf = dur.count();
	}
	else
	{
		realSpf = std::chrono::duration<double>(std::chrono::high_resolution_clock::now()-startClock).count();
	}
	startClock = std::chrono::high_resolution_clock::now();
}

void Window::SwapBuffers()
{
	SDL_GL_SwapWindow(window);
}

double Window::GetSpf()
{
	return realSpf;
}

void Window::ChangeFramerate()
{
	constexpr double framerateList[4] = {
		0.01666, 
		1.0/30.0, 
		1.0/10.0,
		1.0/2.0
	};

	frameRateChoice++;
	if(frameRateChoice >= 4)
		frameRateChoice = 0;
	targetSpf = framerateList[frameRateChoice];
}