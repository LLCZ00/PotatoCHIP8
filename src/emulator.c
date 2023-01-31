/*
* PotatoCHIP-8 - Emulator
*
* CHIP-8 emulation functions
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "chip8.h" // Chip8Memory *MEMORY, RAM_RESERVED_SIZE, TOTAL_RAM
#include "emulator.h"


struct sdl_window {
	SDL_Renderer *renderer;
	SDL_Window *window;
	SDL_Texture *texture;
} emu_window;


/* Initialize CHIP-8 memory and display */
int initialize_emulator(int scale)
{
	if (initialize_memory() != 0) { return -1; }

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		puts("Error initializing SDL.");
		return -1;
	}

	emu_window.window = SDL_CreateWindow("PotatoCHIP8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH * scale, SCREEN_HEIGHT * scale, SDL_WINDOW_SHOWN);
	if (!emu_window.window) {printf("Failed to open window: %s\n", SDL_GetError()); return -1; }

	emu_window.renderer = SDL_CreateRenderer(emu_window.window, -1, SDL_RENDERER_ACCELERATED);
	if (!emu_window.renderer) {printf("Failed to create renderer: %s\n", SDL_GetError()); return -1; }

	emu_window.texture = SDL_CreateTexture(emu_window.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	if (!emu_window.texture) {printf("Failed to create texture: %s\n", SDL_GetError()); return -1; }

	return 0;
}


/* Read bytes from given ROM file into RAM */
int loadROM(const char *path)
{
	FILE *ROMfp = fopen(path, "r");
	if (ROMfp == NULL)
	{
		printf("Error opening file '%s'\n", path);
		return -1;
	}
	fread(MEMORY->ram + MEMORY->pc, 1, TOTAL_RAM - RAM_RESERVED_SIZE, ROMfp); // MEMORY must be initialized first
	fclose(ROMfp);
	return 0;
}


void update()
{
	SDL_UpdateTexture(emu_window.texture, NULL, MEMORY->screen, (sizeof(MEMORY->screen[0]) * SCREEN_WIDTH));
	SDL_RenderClear(emu_window.renderer);
	SDL_RenderCopy(emu_window.renderer, emu_window.texture, NULL, NULL);
	SDL_RenderPresent(emu_window.renderer);
}


int process_input()
{
	int quit = 0;
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type){
			case SDL_QUIT:
				quit = 1;
				break;

			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						quit = 1; break;
					 	
					case SDLK_x:
						MEMORY->keypad[0] = 1; break;

					case SDLK_1:
						MEMORY->keypad[1] = 1; break;

					case SDLK_2:
						MEMORY->keypad[2] = 1; break;

					case SDLK_3:
						MEMORY->keypad[3] = 1; break;

					case SDLK_q:
						MEMORY->keypad[4] = 1; break;

					case SDLK_w:
						MEMORY->keypad[5] = 1; break;

					case SDLK_e:
						MEMORY->keypad[6] = 1; break;

					case SDLK_a:
						MEMORY->keypad[7] = 1; break;

					case SDLK_s:
						MEMORY->keypad[8] = 1; break;

					case SDLK_d:
						MEMORY->keypad[9] = 1; break;

					case SDLK_z:
						MEMORY->keypad[0xA] = 1; break;

					case SDLK_c:
						MEMORY->keypad[0xB] = 1; break;

					case SDLK_4:
						MEMORY->keypad[0xC] = 1; break;

					case SDLK_r:
						MEMORY->keypad[0xD] = 1; break;

					case SDLK_f:
						MEMORY->keypad[0xE] = 1; break;

					case SDLK_v:
						MEMORY->keypad[0xF] = 1; break;
				}
				break;

			case SDL_KEYUP:
				switch (event.key.keysym.sym)
				{
					case SDLK_x:
						MEMORY->keypad[0] = 0; break;

					case SDLK_1:
						MEMORY->keypad[1] = 0; break;

					case SDLK_2:
						MEMORY->keypad[2] = 0; break;

					case SDLK_3:
						MEMORY->keypad[3] = 0; break;

					case SDLK_q:
						MEMORY->keypad[4] = 0; break;

					case SDLK_w:
						MEMORY->keypad[5] = 0; break;

					case SDLK_e:
						MEMORY->keypad[6] = 0; break;

					case SDLK_a:
						MEMORY->keypad[7] = 0; break;

					case SDLK_s:
						MEMORY->keypad[8] = 0; break;

					case SDLK_d:
						MEMORY->keypad[9] = 0; break;

					case SDLK_z:
						MEMORY->keypad[0xA] = 0; break;

					case SDLK_c:
						MEMORY->keypad[0xB] = 0; break;

					case SDLK_4:
						MEMORY->keypad[0xC] = 0; break;

					case SDLK_r:
						MEMORY->keypad[0xD] = 0; break;

					case SDLK_f:
						MEMORY->keypad[0xE] = 0; break;

					case SDLK_v:
						MEMORY->keypad[0xF] = 0; break;
				}
				break;
		}
	}
	return quit;
}


void start_emulator()
{
	
	int quit = 0;
	uint32_t total_cycles = 0;

	while (!quit)
	{
		quit = process_input();
		cycle();

		if(total_cycles % 9 == 0)
		{
			update();
		}
	}

}


/* Destroy/free CHIP-8 memory, displays, etc. */
void shutdown_emulator()
{
	release_memory();
	SDL_DestroyTexture(emu_window.texture);
    SDL_DestroyRenderer(emu_window.renderer);
    SDL_DestroyWindow(emu_window.window);
    SDL_Quit();
}


/* Fetch, decode, execute instruction */
void cycle()
{
	if (MEMORY->delay_timer > 0) { MEMORY->delay_timer--; }
	if (MEMORY->sound_timer > 0) { MEMORY->sound_timer--; }
	if (MEMORY->sound_timer) { /* Beep I didn't impliment */ }

	// Get instruction and increment PC
	MEMORY->ir = (uint16_t)(MEMORY->ram[MEMORY->pc] << 8u | MEMORY->ram[MEMORY->pc + 1]);
	MEMORY->pc += 0x0002;

	// Decode and execute
	execute();
}
