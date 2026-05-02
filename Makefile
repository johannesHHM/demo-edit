# This file is part of dedit - a tool for editing Teeworlds/DDNet demo files
# Copyright (C) 2024, 2026 JHHM
# 
# Dedit is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Dedit is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with dedit.  If not, see <https://www.gnu.org/licenses/>.

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
