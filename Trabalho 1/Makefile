CC = gcc
SOURCES = sources/route.c sources/str.c sources/vehicle.c
INCLUDES = ./includes
BINARY = ./main
PROG = sources/main.c
FLAGS = -Wall -Werror -g -lm -std=c99
PROG_NAME = Trabalho1
VFLAGS = --leak-check=full --track-origins=yes --show-leak-kinds=all -s

all:
	@$(CC) -o $(BINARY) $(PROG) $(SOURCES) -I $(INCLUDES) $(FLAGS)

run:
	@$(BINARY)

valgrind:
	@valgrind $(VFLAGS) $(BINARY)

zip:
	zip -r $(PROG_NAME).zip includes sources main makefile $(DOC)
