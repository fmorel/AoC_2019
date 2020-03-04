all: day_2 day_3 day_4 day_5_9 day_6 day_7 day_8

CFLAGS+= -g

day_2: day_2.o intcode.o
day_5_9: day_5_9.o intcode.o
day_7: day_7.o intcode.o

