CC = gcc
SOURCES = sources/list.c sources/site.c sources/utils.c sources/design.c sources/menuavl.c sources/avl.c
INCLUDES = ./includes
DOC = ./Googlebot_2.0_Doc.pdf ./googlebot.txt
BINARY = ./main
PROG = sources/main.c
FLAGS = -Wall -Werror -g -lm -std=c99
PROG_NAME = Googlebot_2.0
VFLAGS = --leak-check=full --track-origins=yes --show-leak-kinds=all -s

all:
	@$(CC) -o $(BINARY) $(PROG) $(SOURCES) -I $(INCLUDES) $(FLAGS)

run:
	@$(BINARY)

valgrind:
	@valgrind $(VFLAGS) $(BINARY)

zip:
	zip -r $(PROG_NAME).zip includes sources main makefile $(DOC)