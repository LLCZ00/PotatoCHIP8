/*
* PotatoCHIP-8 - CHIP-8
*
* Core CHIP-8 "CPU" Functionality (Memory, registers, etc.)
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "chip8.h" /* Chip8Memory *MEMORY, TOTAL_RAM, STACK_SIZE */

#define START_ADDRESS 512 // Address of first instruction is expected
#define FONTSET_START 0
#define FONTSET_SIZE 80

uint8_t fontset[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


/* Allocate and initialize MEMORY structure pointer.
* - Allocating all memory-related structures to the heap to avoid stack/ptr issues
* - RAM and registers all inside global MEMORY struct, for convenience
*/
int initialize_memory()
{
	/* Allocate MEMORY (declared in chip8.h), set ram and registers pointers */
	MEMORY = malloc(sizeof(struct Chip8Memory));
	if (MEMORY == NULL)
	{
		puts("Error allocating memory structure.");
		return -1;
	}

	/* Allocate and zero ram, registers, etc. */
	memset(MEMORY->ram, 0, TOTAL_RAM);
	memset(MEMORY->registers, 0, sizeof(MEMORY->registers));
	memset(MEMORY->stack, 0, sizeof(MEMORY->stack));
	memset(MEMORY->screen, 0, sizeof(MEMORY->screen));
	memset(MEMORY->keypad, 0, sizeof(MEMORY->keypad));
	MEMORY->delay_timer = 0;
	MEMORY->sound_timer = 0;
	MEMORY->index = 0;
	MEMORY->pc = START_ADDRESS; // Set Program Counter to first instruction of data
	MEMORY->sp = STACK_SIZE - 1; // Set stack pointer to top of the stack (Last element)

	/* Load fontset into reserved area */
	for (uint8_t i = 0; i < FONTSET_SIZE; ++i)
	{
		MEMORY->ram[FONTSET_START + i] = fontset[i];
	}

	time_t t;
	srand((unsigned) time(&t)); // Intialize RNG

	return 0;
}


/* Free global MEMORY struct, registers, and RAM, set pointers to zero */
void release_memory()
{
	if (MEMORY != 0)
	{
		free(MEMORY);
		MEMORY = 0;
	}	
}


/**********************
* CHIP-8 Instructions *
**********************/

/*** Flow Control ***/

// 0x2nnn - Call subroutine (Push current PC to stack, then set to new address)
static void CALL() 
{ 
	MEMORY->stack[MEMORY->sp] = MEMORY->pc;
	MEMORY->sp -= 1; // Cuz the stack grows towards 0
	MEMORY->pc = (MEMORY->ir & 0xFFF); 
}

// 0x00EE - Return from subroutine (Pop last address off stack and set PC to it)
static void RET() 
{ 
	MEMORY->sp += 1;
	MEMORY->pc = MEMORY->stack[MEMORY->sp]; 
} 

// 0x1nnn - Jump to address
static void JMP() { MEMORY->pc = (MEMORY->ir & 0xFFF); }

// 0xBnnn - Jump to address + V0
static void JMP_OFFSET() { MEMORY->pc = ((MEMORY->ir & 0xFFF) + MEMORY->registers[0]); }

// 0x3xkk  - Skip next instruction if equal
static void SKIP_EQ() 
{
	if (MEMORY->registers[(MEMORY->ir >> 8) & 0xF] == (MEMORY->ir & 0xFF)) { MEMORY->pc += 2; }
}

// 0x5xy0 - Skip next instruction if equal (registers)
static void SKIP_EQ_R()
{
	if (MEMORY->registers[(MEMORY->ir >> 8) & 0xF] == MEMORY->registers[(MEMORY->ir >> 4) & 0xF]) { MEMORY->pc += 2; }
}

// 0x4xkk - Skip next instruction if not equal
static void SKIP_N_EQ() 
{
	if (MEMORY->registers[(MEMORY->ir >> 8) & 0xF] != (MEMORY->ir & 0xFF)) { MEMORY->pc += 2; }
}

// 0x9xy0 - Skip next instruction if not equal (registers)
static void SKIP_N_EQ_R()
{
	if (MEMORY->registers[(MEMORY->ir >> 8) & 0xF] != MEMORY->registers[(MEMORY->ir >> 4) & 0xF]) { MEMORY->pc += 2; }
}

// 0xEx9E - Skip next instruction if key is pressed
static void SKIP_KEY() { if (MEMORY->keypad[(MEMORY->ir >> 8) & 0xF]) { MEMORY->pc += 2; } }

// 0xExA1 - Skip next instruction if key is not pressed
static void SKIP_N_KEY() { if (!MEMORY->keypad[(MEMORY->ir >> 8) & 0xF]) { MEMORY->pc += 2; } }

// 0xFx0A - Stop execution until key is pressed
static void WAIT_KEY() 
{ /* Vx = key_value */ 
	for (uint8_t i = 0; i <= 0xF; i++)
	{
		if (MEMORY->keypad[i]) { MEMORY->registers[(MEMORY->ir >> 8) & 0xF] = i; return; }
	}
	MEMORY->pc -= 2;
}

// 0x0000, 0x0nnn
static void NOOP() { /* no-op */ } 


/*** Memory ***/

// 0x6xkk - Load/set register to kk
static void LD_BYTE() { MEMORY->registers[(MEMORY->ir >> 8) & 0xF] = (MEMORY->ir & 0xFF); }

// 0x8xy0
static void LD_R() { MEMORY->registers[(MEMORY->ir >> 8) & 0xF] = MEMORY->registers[(MEMORY->ir >> 4) & 0xF]; }

// 0xFx07 - Load delay timer value into Vx
static void LD_DT() { MEMORY->registers[(MEMORY->ir >> 8) & 0xF] = MEMORY->delay_timer; }

// 0xAnnn - Set I(ndex) register to nnn
static void SET_INDEX() { MEMORY->index = (MEMORY->ir & 0xFFF); }

// 0xFx15 - Delay timer is set to Vx
static void SET_DT() { MEMORY->delay_timer = MEMORY->registers[(MEMORY->ir >> 8) & 0xF]; }

// 0xFx18 - Sound timer is set to Vx
static void SET_ST() { MEMORY->sound_timer = MEMORY->registers[(MEMORY->ir >> 8) & 0xF]; }

// 0xFx29 - Load location of hexadecimal sprite corresponding to x into I
static void LOAD_SPRITE() { MEMORY->index = FONTSET_START + (5 * (MEMORY->ir >> 8) & 0xF); }

// 0xFx55 - Store registers in memory (Copies values from V0-Vx into memory, starting at I)
static void STORE_REGISTERS() 
{ 
	for (uint8_t i = 0; i <= ((MEMORY->ir >> 8) & 0xF); i++)
	{
		MEMORY->ram[MEMORY->index + i] = MEMORY->registers[i];
	}
}

// 0xFx65 - Load values from memory into registers (I into V0-Vx)
static void LOAD_REGISTERS() 
{
	for (uint8_t i = 0; i <= ((MEMORY->ir >> 8) & 0xF); i++)
	{
		MEMORY->registers[i] = MEMORY->ram[MEMORY->index + i];
	}
}

// 0xFx33 - Store BCD representation of Vx in memory locations I, I+1, and I+2
static void STORE_BCD() 
{
	uint8_t value = (MEMORY->ir >> 8) & 0xF;
	MEMORY->ram[MEMORY->index + 2] = value % 10;
	value /= 10;
	MEMORY->ram[MEMORY->index + 1] = value % 10;
	value /= 10;
	MEMORY->ram[MEMORY->index] = value % 10;
}


/*** Arithmetic ***/

// 0xCxkk - Random number (0-255) & kk
static void RAND() { MEMORY->registers[(MEMORY->ir >> 8) & 0xF] = ((MEMORY->ir & 0xFF) & (uint8_t)rand()); }

// 0x7xkk - Add (VF (carry) flag not affected)
static void ADD() { MEMORY->registers[(MEMORY->ir >> 8) & 0xF] += (MEMORY->ir & 0xFF); }

// 0xFx1E - ADD (Index)
static void I_ADD() { MEMORY->index += MEMORY->registers[(MEMORY->ir >> 8) & 0xF]; }

// 0x8xy4 - Bitwise ADD (Carry stored in VF)
static void BIT_ADD() 
{
	uint16_t sum = MEMORY->registers[(MEMORY->ir >> 8) & 0xF] + MEMORY->registers[(MEMORY->ir >> 4) & 0xF];
	MEMORY->registers[(MEMORY->ir >> 8) & 0xF] = (sum & 0xFF);
	MEMORY->registers[0xF] = (sum >> 8) ? 0x01 : 0x00;
}

// 0x8xy1 - Bitwise OR
static void BIT_OR() { MEMORY->registers[(MEMORY->ir >> 8) & 0xF] |= MEMORY->registers[(MEMORY->ir >> 4) & 0xF]; }

// 0x8xy2 - Bitwise AND
static void BIT_AND() { MEMORY->registers[(MEMORY->ir >> 8) & 0xF] &= MEMORY->registers[(MEMORY->ir >> 4) & 0xF]; }

// 0x8xy3 - Bitwise XOR
static void BIT_XOR() { MEMORY->registers[(MEMORY->ir >> 8) & 0xF] ^= MEMORY->registers[(MEMORY->ir >> 4) & 0xF]; }

// 0x8xy5 - Bitwise SUB (If Vx > Vy, then VF is set to 1, otherwise 0)
static void BIT_SUB()
{
	MEMORY->registers[0xF] = (MEMORY->registers[(MEMORY->ir >> 8) & 0xF] > MEMORY->registers[(MEMORY->ir >> 4) & 0xF]) ? 0x01 : 0x00;
	MEMORY->registers[(MEMORY->ir >> 8) & 0xF] -= MEMORY->registers[(MEMORY->ir >> 4) & 0xF];
}

// 0x8xy6 - Bitwise SHR (Divide Vx by 2)
static void BIT_SHR() 
{ /* Store LSB in VF, Vx >>= 1 (if y != 0, Vx = Vy first) */ 
	MEMORY->registers[0xF] = MEMORY->registers[(MEMORY->ir >> 8) & 0xF] & 0x1u;
	MEMORY->registers[(MEMORY->ir >> 8) & 0xF] >>= 1;
}

// 0x8xy7 - Bitwise SUBN If Vy > Vx, then VF is set to 1, otherwise 0
static void BIT_SUBN()
{
	MEMORY->registers[0xF] = (MEMORY->registers[(MEMORY->ir >> 4) & 0xF] > MEMORY->registers[(MEMORY->ir >> 8) & 0xF]) ? 0x01 : 0x00;
	MEMORY->registers[(MEMORY->ir >> 8) & 0xF] = MEMORY->registers[(MEMORY->ir >> 4) & 0xF] - MEMORY->registers[(MEMORY->ir >> 8) & 0xF];
}

// 0x8xyE - Bitwise SHL (Multiply Vx by 2)
static void BIT_SHL() 
{ /* Store MSB in VF, Vx >>= 1 (if y != 0, Vx = Vy first) */ 
	MEMORY->registers[0xF] = (MEMORY->registers[(MEMORY->ir >> 8) & 0xF] & 0x80u) >> 7u;
	MEMORY->registers[(MEMORY->ir >> 8) & 0xF] <<= 1;
}


/*** Display ***/

// 0x00E0 - Clear screen
static void CLS() { memset(MEMORY->screen, 0, sizeof(MEMORY->screen)); }

// 0xDxyn - Draw n-byte spirit stored in I(ndex) at (Vx, Vy)
static void DRAW() 
{
	uint8_t height = MEMORY->ir & 0x000Fu;
	uint8_t xPos = MEMORY->registers[(MEMORY->ir >> 8) & 0xF] % SCREEN_WIDTH;
	uint8_t yPos = MEMORY->registers[(MEMORY->ir >> 4) & 0xF] % SCREEN_HEIGHT;
	MEMORY->registers[0xF] = 0;
	for (unsigned int row = 0; row < height; ++row)
	{
		uint8_t spriteByte = MEMORY->ram[MEMORY->index + row];

		for (unsigned int col = 0; col < 8; ++col)
		{
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &MEMORY->screen[(yPos + row) * SCREEN_WIDTH + (xPos + col)];

			// Sprite pixel is on
			if (spritePixel)
			{
				// Screen pixel also on - collision
				if (*screenPixel == 0xFFFFFFFF)
				{
					MEMORY->registers[0xF] = 1;
				}

				// Effectively XOR with the sprite pixel
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}

static void _0___();
static void _8___();
static void _E___();
static void _F___();

static void (*opcode_0[])() = { CLS, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, RET, NOOP };

static void (*opcode_8[])() = { LD_R, BIT_OR, BIT_AND, BIT_XOR, BIT_ADD, BIT_SUB, BIT_SHR, BIT_SUBN, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, BIT_SHL, NOOP };

static void (*opcode_E[])() = {  NOOP, SKIP_N_KEY, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, SKIP_KEY, NOOP };

static void (*opecode_F[])() = { NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, LD_DT, NOOP, NOOP, WAIT_KEY, NOOP, NOOP, NOOP, NOOP, NOOP,
 							     NOOP, NOOP, NOOP, NOOP, NOOP, SET_DT, NOOP, NOOP, SET_ST, NOOP, NOOP, NOOP, NOOP, NOOP, I_ADD, NOOP,
							     NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, LOAD_SPRITE, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP,
							     NOOP, NOOP, NOOP, STORE_BCD, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP,
 							     NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP,
 							     NOOP, NOOP, NOOP, NOOP, NOOP, STORE_REGISTERS, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP,
							     NOOP, NOOP, NOOP, NOOP, NOOP, LOAD_REGISTERS, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP,
							     NOOP, NOOP, NOOP, NOOP, NOOP, LOAD_REGISTERS, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, NOOP };


static void (*_exec[])() = {_0___, JMP, CALL, SKIP_EQ, SKIP_N_EQ, SKIP_EQ_R, LD_BYTE, ADD, _8___, SKIP_N_EQ_R, SET_INDEX, JMP_OFFSET, RAND, DRAW, _E___, _F___};
static void _0___() { (*opcode_0[MEMORY->ir & 0x000F])(); }
static void _8___() { (*opcode_8[MEMORY->ir & 0x000F])(); }
static void _E___() { (*opcode_E[MEMORY->ir & 0x000F])(); }
static void _F___() { (*opecode_F[MEMORY->ir & 0x00FF])(); }

void execute()
{
	(*_exec[MEMORY->ir >> 12])();
}

