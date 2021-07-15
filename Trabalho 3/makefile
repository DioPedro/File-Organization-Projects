CC = gcc
SOURCES = sources/route.c sources/vehicle.c sources/utils.c sources/menu.c sources/btree.c sources/intersect.c
INCLUDES = ./includes
BINARY = ./programaTrab
PROG = sources/main.c
FLAGS = -Wall -Werror -g -lm
PROG_NAME = Trabalho3
VFLAGS = --leak-check=full --track-origins=yes --show-leak-kinds=all -s

all:
	@$(CC) -o $(BINARY) $(PROG) $(SOURCES) -I $(INCLUDES) $(FLAGS)

run:
	@$(BINARY)

valgrind:
	@valgrind $(VFLAGS) $(BINARY)

zip:
	zip -r $(PROG_NAME).zip includes sources main makefile $(DOC)

clean:
	@rm *.bin $(BINARY)