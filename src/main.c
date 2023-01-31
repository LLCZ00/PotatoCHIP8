/*
* PotatoCHIP-8 - Main
*
* Main function/Argument Parsing
*/

/* TODO:

- Add additional arguments to argparse function as needed

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "emulator.h" // loadROM(), initialize_emulator(), shutdown_emulator();
#include "debugger.h"

static const char *VERSION = "1.0.0";
static const char *USAGE = "Usage: ./potatoCHIP8 [-h] [--debug] [--disas] ROM";
static const char *HELP[] = 
{
	"",
	"Description: CHIP-8 emulator and graphical debugger",
	"",
	"Positional:",
	"\tROM             ROM file to run/disassemble (.ch8)",
	"",
	"Options:",
	"\t-h, --help      Show usage (this page)",
	"\t-v, --version   Show version and exit",
	"\t--debug         Run graphical debugger alongside ROM",
	"\t--disas         Print disassembly of ROM and exit",
	0
};

static struct arguments{ // Arguments to be populated by argparse()
	int debug;
	int disas;
	char *rom;
} args={0,0,0};

static void argparse(int argc, char **argv) // Parse command-line arguments
{
	if (argc <= 1) // Print usage and exit if no arguments
	{
		puts(USAGE);
		exit(0);
	}

	int index = 1;
	while (1)
	{
		if (argv[index] == NULL){ break; }

		/* Optional Arguments */

        // Help menu
        else if ((strncmp(argv[index], "-h\0", 3) == 0) || (strncmp(argv[index], "--help\0", 7) == 0))
        {
        	puts(USAGE);
            for (int i = 0; HELP[i]; i++)
            {
                printf("%s\n", HELP[i]);
            }
            exit(0);
        }
        // Version
        else if ((strncmp(argv[index], "-v\0", 3) == 0) || (strncmp(argv[index], "--version\0", 10) == 0))
        {
        	printf("PotatoCHIP-8 v%s\n", VERSION);
        	exit(0);
        }
        // Disassemble
        else if ((strncmp(argv[index], "--disas\0", 8) == 0))
        {
        	if (args.disas) { break; };
        	args.disas = 1;
        	index += 1;
        	continue;
        }
        // Debug
        else if ((strncmp(argv[index], "--debug\0", 8) == 0))
        {
        	if (args.disas) { break; };
        	args.debug = 1;
        	index += 1;
        	continue;
        }

        /* Positional Argument (ROM) */

        else // File can be anywhere in the argument array
        {
        	if (args.rom != 0) { printf("Unknown argument '%s'\n", argv[index]); exit(-1); }

        	if (access(argv[index], F_OK) == 0) // access from unistd.h
        	{
        		args.rom = argv[index]; // Set rom to the original argv const ptr
        		index += 1;
        		continue;
        	} 
        	else { printf("File not found '%s'\n", argv[index]); exit(-1); }
        }

	}

	/* Validate Arguments */
	if (args.rom == 0) 
	{
		puts("Argument required 'ROM'\n");
		exit(-1);
	}
}


int main(int argc, char **argv)
{
	argparse(argc, argv); // Either gathers arguments successfully or exits

	if (args.disas) { disassemble_file(args.rom); return 0; }

	if (initialize_emulator(10) != 0) { return -1; }

	if (loadROM(args.rom) != 0) { return -1; }

	if (args.debug) { cmd_debug(); }
	else { start_emulator(); }

	shutdown_emulator();
	puts("\nPotatoCHIP-8 exited gracefully.");
	return 0;
}


