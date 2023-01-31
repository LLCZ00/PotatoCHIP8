/*
* PotatoCHIP-8 - Debugger Header
*/

/* PUBLIC FUNCTIONS
   - dump_memory()
   - dump_stack()
   - dump_registers()
   - dump_all()

   PUBLIC STRUCTS
   - doink

   PUBLIC VARIABLES
   - donk
*/

#ifndef POTATOCHIP_DEBUGGER
#define POTATOCHIP_DEBUGGER

#include <stdint.h>
#include <stddef.h>


/* Dump/print RAM (values and offset) */
void dump_memory(uint16_t start_offset, uint16_t stop_offset); 

/* Disassemble single instruction and return result string in given buffer */
void disassemble_instruction(char results_buffer[], size_t buffer_size, uint16_t instruction);

/* Print disassembly of given ROM */
void disassemble_file(const char *path);

/* Ncurses debugger */
void cmd_debug();

#endif // POTATOCHIP_DEBUGGER
