CC=gcc
CFALGS=-Wall -O2 -std=gnu99
IFLAGS=-Isrc
LDFLAGS=-ljansson -lcurl
SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)
EXEC=fbdl

all:	$(EXEC)

fbdl:	main.c $(OBJ)
		$(CC) -o $@ $^ $(CFALGS) $(LDFLAGS) $(IFLAGS)

%.o:	%.c %.h
		$(CC) -c $< -o $@ $(CFALGS) $(LDFLAGS)

clean:	
		rm -vfr $(OBJ)

mrproper:	clean
			rm -vfr $(EXEC)