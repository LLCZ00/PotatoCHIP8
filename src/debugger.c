/*
* PotatoCHIP-8 - Debugger
*
* Functions for Testing/Debugging
*
* Dumping functions pull the global MEMORY 
* struct directly, so there's no need to pass
* it as an argument.
*/

/* TODO:

- GUI
- Disassembler function

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <ncurses.h>
#include "debugger.h"
#include "chip8.h" // Chip8Memory *MEMORY, RAM_DATA_SIZE, TOTAL_RAM, STACK_SIZE
#include "emulator.h"


#define MAX_CMD_SIZE 50

#define ROWLENGTH 16

/* Dump RAM offset: values (upper limit exclusionary) */
void dump_memory(uint16_t start_offset, uint16_t stop_offset)
{
	if ((start_offset >= TOTAL_RAM) || (stop_offset > TOTAL_RAM))
	{
		puts("Start/stop offset out of memory range");
		return;
	}

	uint8_t *offset_memory = MEMORY->ram + start_offset;

	for (uint16_t i = 0; i < stop_offset-start_offset; i++)
	{		
		if ((i % ROWLENGTH) == 0)
		{
			printf("\n0x%03X: ", start_offset + i);
		}
		printf("%02x ", offset_memory[i]);
	}
	puts("");
}

/* Disassemble single instruction and return result string in given buffer */
void disassemble_instruction(char results_buffer[], size_t buffer_size, uint16_t instruction)
{
	memset(results_buffer, 0, buffer_size);
	char operands[7] = {0};
	snprintf(results_buffer, buffer_size, "Unknown instruction: 0x%04X", instruction);	
	switch(instruction >> 12) {

		case 0:
			switch(instruction & 0xFF){
				case 0xE0:
					snprintf(results_buffer, buffer_size, "CLS");
					break;
				case 0xEE:
					snprintf(results_buffer, buffer_size, "RET");
					break;
				case 0x00:
					snprintf(results_buffer, buffer_size, "NOP");
					break;
			}
			break;
		case 1:
			snprintf(results_buffer, buffer_size, "JP 0x%03X", (instruction & 0xFFF));
			break;
		case 2:
			snprintf(results_buffer, buffer_size, "CALL 0x%03X", (instruction & 0xFFF));
			break;
		case 3:
			snprintf(results_buffer, buffer_size, "SE V%X, 0x%02X", ((instruction >> 8) & 0xF), (instruction & 0xFF));
			break;
		case 4:
			snprintf(results_buffer, buffer_size, "SNE V%X, 0x%02X", ((instruction >> 8) & 0xF), (instruction & 0xFF));
			break;
		case 5:
			snprintf(results_buffer, buffer_size, "SE V%X, V%X", ((instruction >> 8) & 0xF), ((instruction >> 4) & 0xF));
			break;
		case 6:
			snprintf(results_buffer, buffer_size, "LD V%X, 0x%02X", ((instruction >> 8) & 0xF), (instruction & 0xFF));
			break;
		case 7:
			snprintf(results_buffer, buffer_size, "ADD V%X, 0x%02X", ((instruction >> 8) & 0xF), (instruction & 0xFF));
			break;
		case 8:
			snprintf(operands, 7, "V%X, V%X", ((instruction >> 8) & 0xF), ((instruction >> 4) & 0xF));
			switch(instruction & 0xF){
				case 0:
					snprintf(results_buffer, buffer_size, "LD %s", operands);
					break;
				case 1:
					snprintf(results_buffer, buffer_size, "OR %s", operands);
					break;
				case 2:
					snprintf(results_buffer, buffer_size, "AND %s", operands);
					break;
				case 3:
					snprintf(results_buffer, buffer_size, "XOR %s", operands);
					break;
				case 4:
					snprintf(results_buffer, buffer_size, "ADD %s", operands);
					break;
				case 5:
					snprintf(results_buffer, buffer_size, "SUB %s", operands);
					break;
				case 6:
					snprintf(results_buffer, buffer_size, "SHR V%X {, V%X}", ((instruction >> 8) & 0xF), ((instruction >> 4) & 0xF));
					break;
				case 7:
					snprintf(results_buffer, buffer_size, "SUBN %s", operands);
					break;
				case 0xE:
					snprintf(results_buffer, buffer_size, "SHL V%X {, V%X}", ((instruction >> 8) & 0xF), ((instruction >> 4) & 0xF));
					break;
			}
			break;
		case 9:
			snprintf(results_buffer, buffer_size, "SNE V%X, V%X", ((instruction >> 8) & 0xF), ((instruction >> 4) & 0xF));
			break;
		case 0xA:
			snprintf(results_buffer, buffer_size, "LD I, 0x%03X", (instruction & 0xFFF));
			break;
		case 0xB:
			snprintf(results_buffer, buffer_size, "JP 0x%03X + V0", (instruction & 0xFFF));
			break;
		case 0xC:
			snprintf(results_buffer, buffer_size, "RND V%X, 0x%02X", ((instruction >> 8) & 0xF), (instruction & 0xFF));
			break;
		case 0xD:
			snprintf(results_buffer, buffer_size, "DRW V%X, V%X, 0x%X", ((instruction >> 8) & 0xF), ((instruction >> 4) & 0xF), (instruction & 0xF));
			break;
		case 0xE:
			switch(instruction & 0xFF){
				case 0x9E:
					snprintf(results_buffer, buffer_size, "SKP V%X", ((instruction >> 8) & 0xF));
					break;
				case 0xA1:
					snprintf(results_buffer, buffer_size, "SKNP V%X", ((instruction >> 8) & 0xF));
					break;
			}
			break;
		case 0xF:
			snprintf(operands, 3, "V%X", ((instruction >> 8) & 0xF));
			switch(instruction & 0xFF){
				case 0x07:
					snprintf(results_buffer, buffer_size, "LD %s, DT", operands);
					break;
				case 0x0A:
					snprintf(results_buffer, buffer_size, "LD %s, K", operands);
					break;
				case 0x15:
					snprintf(results_buffer, buffer_size, "LD DT, %s", operands);
					break;
				case 0x18:
					snprintf(results_buffer, buffer_size, "LD ST, %s", operands);
					break;
				case 0x1E:
					snprintf(results_buffer, buffer_size, "ADD I, %s", operands);
					break;
				case 0x29:
					snprintf(results_buffer, buffer_size, "LD F, %s", operands);
					break;
				case 0x33:
					snprintf(results_buffer, buffer_size, "LD B, %s", operands);
					break;
				case 0x55:
					snprintf(results_buffer, buffer_size, "LD [I], %s", operands);
					break;
				case 0x65:
					snprintf(results_buffer, buffer_size, "LD %s, [I]", operands);
					break;
			}
			break;
	}

	results_buffer[buffer_size] = '\0'; // Manually adding null-terminator just in case
}

/* Print disassembly of ROM at given path*/
void disassemble_file(const char *path)
{
	uint8_t read_buffer[4096];
	memset(read_buffer, 0, 4096);

	FILE *ROMfp = fopen(path, "r");
	if (ROMfp == NULL)
	{
		printf("Error opening file '%s'\n", path);
		return;
	}
	size_t bytes_read = fread(read_buffer, 1, 4096, ROMfp); // MEMORY must be initialized first
	fclose(ROMfp);

	if (bytes_read < 2)
	{
		printf("Error reading bytes from '%s'\n", path);
		return;
	}

	char mnemonic_buffer[30];
	size_t buffer_size = sizeof(mnemonic_buffer);
	for (int i = 0; i < (int)bytes_read; i+=2)
	{
		disassemble_instruction(mnemonic_buffer, buffer_size, (uint16_t)(read_buffer[i] << 8u | read_buffer[i + 1]));
		printf("0x%03X: %s\n", 0x200+i, mnemonic_buffer);
	}
}


static WINDOW *create_window(int height, int width, int start_y, int start_x)
{
    WINDOW *local_win;
    local_win = newwin(height, width, start_y, start_x);
    box(local_win, 0 , 0);
    wrefresh(local_win);
    return local_win;
}

static void destroy_window(WINDOW *local_win)
{
    wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    wrefresh(local_win);
    delwin(local_win);
}


static void update_registers(WINDOW *rwin)
{
	int row = 1;
	int column = 1;

	wmove(rwin, row, column);
	wclrtoeol(rwin);
	wprintw(rwin, "PC: 0x%04X", MEMORY->pc);

	wmove(rwin, row+1, column);
	wclrtoeol(rwin);
	wprintw(rwin, "I : 0x%04X", MEMORY->index);

	wmove(rwin, row+2, column);
	wclrtoeol(rwin);
	wprintw(rwin, "SP: 0x%X", MEMORY->sp);

	for (int i = 0; i < 16; i++)
	{
		wmove(rwin, row+i+3, column);
		wclrtoeol(rwin);
		wprintw(rwin, "V%X: 0x%02X", i, MEMORY->registers[i]);
	}
	box(rwin, 0 , 0);
	wrefresh(rwin);
}

static void update_stack(WINDOW *swin)
{
	int row = 1;
	int column = 1;

	for (int i = 0; i < STACK_SIZE; i++)
	{
		wmove(swin, row+i, column);
		wclrtoeol(swin);
		if (i == MEMORY->sp)
		{
			wprintw(swin, "0x%X: 0x%04X<-SP", i, MEMORY->stack[i]);
		}
		else { wprintw(swin,"0x%X: 0x%04X", i, MEMORY->stack[i]); }		
	}
	box(swin, 0 , 0);
	wrefresh(swin);
}


static void update_disas(WINDOW *dwin)
{
	int row = 1;
	int column = 1;
	char results_buffer[25];
	size_t results_size = sizeof(results_buffer);

	for (int i = 0; i < 50; i+=2)
	{
		wmove(dwin, row+(i/2), column);
		wclrtoeol(dwin);
		disassemble_instruction(results_buffer, results_size, (uint16_t)(MEMORY->ram[MEMORY->pc + i] << 8u | MEMORY->ram[MEMORY->pc + i + 1]));
		wprintw(dwin, "0x%03X: %s ; 0x%04X", MEMORY->pc + i, results_buffer, (uint16_t)(MEMORY->ram[MEMORY->pc + i] << 8u | MEMORY->ram[MEMORY->pc + i + 1]));
	}
	box(dwin, 0 , 0);
	wrefresh(dwin);
}


void cmd_debug()
{
	initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    int ch;
    char command_string[MAX_CMD_SIZE];
    int command_length;
    char last_command[MAX_CMD_SIZE];
    memset(last_command, 0, sizeof(last_command));

    int quit_loop = 0;
    int row, column;

    char *cmd_prefix = "> ";
    int prefix_length = strlen(cmd_prefix) + 2;

    char *title = "~ PotatoCHIP8 Debugger ~";
    mvprintw(0, (COLS - strlen(title))/2, "%s", title);
    refresh();

    WINDOW *disas_window = create_window(28, 50, 2, 2);
	update_disas(disas_window);
    WINDOW *register_window = create_window(21, 15, 2, 54);
    update_registers(register_window);
	WINDOW *stack_window = create_window(21, 18, 2, 70);
	update_stack(stack_window);
	

    while(!quit_loop)
    {
    	move(30, 2);
    	clrtoeol();
    	printw("%s", cmd_prefix);
    	memset(command_string, 0, sizeof(command_string));
        command_length = 0;
        refresh();

    	while(1) // Input loop
    	{
    		ch = getch();
    		switch(ch)
    		{
    		case 3: // Ctrl-c
    			quit_loop = 1;
    			break;
			case KEY_LEFT:
                getyx(stdscr,row,column);
                if ((column-prefix_length) > 0) { move(row, column - 1); } // Move cursor left, but not passed "> " thing
                break;
            case KEY_RIGHT:
                getyx(stdscr,row,column);
                if ((column-prefix_length) < (command_length)) { move(row, column + 1); } // Move cursor right, but not more than 1 passed currently typed characters
                break;
            case KEY_UP:
                if (strlen(last_command))
                {
                    getyx(stdscr,row,column);
                    move(row, prefix_length);
                    clrtoeol();
                    command_length = strlen(last_command);
                    printw("%s", last_command);
                }
                break;
            case KEY_DOWN:
                break;
    		case 0x107: // backspace
	            getyx(stdscr,row,column);
	            if ((column-prefix_length) > 0)
	            {
	                move(row, column - 1);
	                delch();
	                command_length -= 1;
	            }
	            break;
	        case 0xA: // Enter key/newline
	            getyx(stdscr,row,column);
	            move(row, command_length + prefix_length);
	            if (command_length)
	            {
	                for (int pos = 0; pos <= command_length && pos < MAX_CMD_SIZE; pos++)
	                {
	                    command_string[pos] = mvinch(row, pos+prefix_length) & A_CHARTEXT;
	                }
	                command_string[command_length] = '\0';

	                strncpy(last_command, command_string, MAX_CMD_SIZE);
	            }
	            else if (strlen(last_command)) // Run last command if available and no command given
	            {
	                command_length = strlen(last_command);
	                strncpy(command_string, last_command, MAX_CMD_SIZE); 
	            }
	            break;
	        default:
	            getyx(stdscr,row,column);
	            insch(ch);
	            move(row, column + 1);
	            command_length += 1;
    		}
			refresh();
            if (ch == 3) { break; }
            if (ch == 0xA) { break; }
    	}
 
 		if ((strncmp(command_string, "exit\0", 5) == 0) || (strncmp(command_string, "quit\0", 5) == 0))
 		{
 			quit_loop = 1;
 		}
    	else if ((strncmp(command_string, "s\0", 2) == 0) || (strncmp(command_string, "step\0", 5) == 0))
    	{
    		cycle();
	        update();
	        update_disas(disas_window);
	        update_registers(register_window);
	        update_stack(stack_window);
    	}
    }

    destroy_window(stack_window);
    destroy_window(register_window);
    destroy_window(disas_window);
    endwin();
}


