.PHONY: clean potatoCHIP8

C_SOURCES = $(wildcard src/*.c src/*.h)

CC = gcc


potatoCHIP8: $(C_SOURCES)
	$(CC) -o $@ $^ -lSDL2 -lncurses

clean:
	rm -f ./potatoCHIP8