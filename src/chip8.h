/*
* PotatoCHIP-8 - CHIP-8 Header
*
* Core CHIP-8 Declarations
*/

/* PUBLIC FUNCTIONS
   - initialize_memory()
   - release_memory()

   PUBLIC STRUCTS
   - Chip8Memory

   PUBLIC VARIABLES
   - Chip8Memory *MEMORY
*/

#ifndef POTATOCHIP_CHIP8
#define POTATOCHIP_CHIP8

#include <stdint.h>

#define TOTAL_RAM 4096
#define STACK_SIZE 16
#define RAM_RESERVED_SIZE 512 // Memory reserved for CHIP-8 interpreter
#define SCREEN_HEIGHT 32u
#define SCREEN_WIDTH 64u

/*
* MEMORY ptr is declared so it can be used by any file that
* includes chip8.h. MEMORY is allocated by initialize_memory(),
* and freed by release_memory()
*/
struct Chip8Memory{ 
	uint8_t delay_timer;
	uint8_t sound_timer;
	uint8_t registers[16]; // General registers (V0 - VF)
	uint16_t index;      // I (index) register, holds memory addresses 
	uint16_t pc;         // Program Counter, stores address of next instruction to execute
	uint16_t ir;         // Instruction register, stores instruction currently being executed
	uint8_t sp;          // Stack pointer
	uint16_t stack[STACK_SIZE]; // Stack, used for storing return addresses (LIFO, high to low)
	uint8_t ram[TOTAL_RAM];
	uint32_t screen[SCREEN_WIDTH * SCREEN_HEIGHT];
	uint8_t keypad[16];
} *MEMORY; 


/* Initialize RAM and registers, allocate MEMORY ptr */
int initialize_memory();

/* Free global MEMORY struct, registers, and RAM  */
void release_memory();

void execute();

#endif // POTATOCHIP_CHIP8
