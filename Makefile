all: day_2 day_3 day_4 day_5_9 day_6 day_7 day_8 day_10 day_11 day_12 day_13 day_14

CFLAGS+= -g -lm

SDL_FLAGS=$$(sdl2-config --libs)

day_2: day_2.o intcode.o
day_5_9: day_5_9.o intcode.o
day_7: day_7.o intcode.o
day_11: day_11.o intcode.o

day_13: day_13.o intcode.o
	$(CC) $(SDL_FLAGS) $+ -o $@

