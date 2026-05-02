.POSIX:

CC       = c99
CFLAGS   = -O2 -Wall -Wextra -pedantic-errors
CPPFLAGS = -Iinc

TARGET = dedit
OBJDIR = obj

SRC = \
	src/args.c \
	src/chat.c \
	src/commands.c \
	src/demo.c \
	src/huffman.c \
	src/main.c \
	src/pack.c

OBJ = \
	obj/args.o \
	obj/chat.o \
	obj/commands.o \
	obj/demo.o \
	obj/huffman.o \
	obj/main.o \
	obj/pack.o

all: $(OBJDIR) $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)

format:
	clang-format -i --style=Microsoft src/*.c
	clang-format -i --style=Microsoft inc/*.h

$(OBJDIR):
	mkdir -p $(OBJDIR)

obj/args.o: src/args.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/args.c -o obj/args.o

obj/chat.o: src/chat.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/chat.c -o obj/chat.o

obj/commands.o: src/commands.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/commands.c -o obj/commands.o

obj/demo.o: src/demo.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/demo.c -o obj/demo.o

obj/huffman.o: src/huffman.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/huffman.c -o obj/huffman.o

obj/main.o: src/main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/main.c -o obj/main.o

obj/pack.o: src/pack.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/pack.c -o obj/pack.o

.PHONY: all clean format
