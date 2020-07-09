#include <SDL.h>
#include "Renderer.h"
#include "Shader.h"
#include "Camera.h"
#include <algorithm>

#include "Crate.h"
#include "Floor.h"
#include "Pillar.h"

int main(int argc, char** argv)
{
	// Setup SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", SDL_GetError(), nullptr);
		return -1;
	}

	// Create the window
	int window_width = 800;
	int window_height = 600;

	SDL_Window* window = SDL_CreateWindow("DirectX", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE);
	if (window == nullptr)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", SDL_GetError(), nullptr);
		return -1;
	}

	// Create renderer
	Renderer* renderer = new Renderer(window);
	if (!renderer->Init())
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Renderer::Init failed", nullptr);
		return -1;
	}

	// Camera
	Camera* camera = new Camera(800, 600);

	// Create shader
	Shader* shader = new Shader(renderer);
	if (!shader->Create())
		return -1;

	// Models
	Crate* crate = new Crate(renderer);
	if (!crate->Load())
		return -1;

	Floor* floor = new Floor(renderer);
	if (!floor->Load())
		return -1;

	Pillar* pillarLeft = new Pillar(renderer);
	if (!pillarLeft->Load())
		return -1;
	
	Pillar* pillarRight = new Pillar(renderer);
	if (!pillarRight->Load())
		return -1;

	pillarLeft->Position.x = -3.0f;
	pillarRight->Position.x = 3.0f;

	// Main loop
	bool running = true;
	while (running)
	{
		SDL_Event e;
		if (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				running = false;
				break;

			case SDL_WINDOWEVENT:
				switch (e.window.event)
				{
				case SDL_WINDOWEVENT_RESIZED:
					renderer->Resize(e.window.data1, e.window.data2);
					camera->Resize(e.window.data1, e.window.data2);
					break;
				}

			case SDL_MOUSEMOTION:
				if (e.motion.state == SDL_BUTTON_LMASK)
				{
					camera->Update((float)e.motion.xrel, (float)e.motion.yrel);
				}
				break;

			case SDL_MOUSEWHEEL:
				camera->UpdateFov((int)e.wheel.y);
				break;

			case SDL_KEYDOWN:
				if (e.key.keysym.scancode == SDL_SCANCODE_1)
				{
					static bool enable_wireframe = false;
					enable_wireframe = !enable_wireframe;
					renderer->EnableWireframe(enable_wireframe);
				}
				break;
			}
		}
		else
		{
			renderer->Clear();

			shader->Use();
			crate->Render(camera);
			floor->Render(camera);

			pillarLeft->Render(camera);
			pillarRight->Render(camera);

			renderer->Render();
		}
	}

	// Cleanup
	renderer->Quit();
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}