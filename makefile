CC = gcc
CFLAGS = -Wall
DEPS = globalvars.h fileHandling.h errorHandling.h endianness.h encoding.h test.h runPNG.h
OBJ = fileHandling.o errorHandling.o endianness.o encoding.o test.o runPNG.o

all:test.exe

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

test.exe: $(OBJ)
	gcc $(CFLAGS) -o $@ $^ -lpng