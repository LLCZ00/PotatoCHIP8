/*
* PotatoCHIP-8 - Emulator Header
*
* Function Declarations
*/


#ifndef POTATOCHIP_EMULATOR
#define POTATOCHIP_EMULATOR

#include <stdint.h>


/* Initialize CHIP-8 memory and display */
int initialize_emulator(int scale);

/* Read bytes from CHIP-8 ROM file into initialized RAM */
int loadROM(const char *path);

void update();

/* Start main loop */
void start_emulator();

/* Destroy/free CHIP-8 memory, displays, etc. */
void shutdown_emulator();

/* Fetch, decode, execute instruction */
void cycle();

#endif // POTATOCHIP_EMULATOR
