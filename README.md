# PotatoCHIP8
### _CHIP-8 Emulator_
*PotatoCHIP8* is a CHIP-8 emulator that runs about as well as its namesake, the mighty potato. This project was created as a first step into the world of emulator development (but mostly just for fun). *PotatoCHIP8* includes a disassmbler, graphical debugger, and (of course) an emulator.

*PotatoCHIP8* is only compatible with Linux, and relies on SDL2 and ncurses for the various graphical displays.

## Known Issues & TODO
- Debugger currently just steps, and overall isn't as good as I'd like it to be
- Fix timing issues
- Ocassional input errors (differs between ROMs)
- I foolishly didn't name the instuction functions after their opcodes
- The VM I used to develop this has no audio, and so this has no audio
