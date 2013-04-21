CC=g++

CFLAGS=-Wall -lpthread

OBJECTS=src/math_engine.o src/commandline.o src/core.o src/helper.o src/irc.o src/output.o src/tools.o src/ai.o

SOURCES=src/math_engine.cpp src/commandline.cpp src/core.cpp src/helper.cpp src/irc.cpp src/output.cpp src/tools.cpp src/ai.cpp

NAME=helper

%.o: %.c
	$(CC) $(CFLAGS) -O2 -c $<

helper: $(OBJECTS)
	$(CC) -o $(NAME) -O2  $(OBJECTS)  $(CFLAGS)

clean:
	rm src/*.o
