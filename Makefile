# the compiler: gcc for C program, define as g++ for C++
CC = g++

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall -std=c++11

default = all

all: hospitalA hospitalB hospitalC scheduler client

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c


hospitalA: hospitalA.cpp hospital.o network.o
	$(CC) $(CFLAGS) -o hospitalA hospitalA.cpp hospital.o network.o

hospitalB: hospitalB.cpp hospital.o network.o
	$(CC) $(CFLAGS) -o hospitalB hospitalB.cpp hospital.o network.o

hospitalC: hospitalC.cpp hospital.o network.o
	$(CC) $(CFLAGS) -o hospitalC hospitalC.cpp hospital.o network.o


hospital.o: hospital.cpp hospital.hpp
	$(CC) $(CFLAGS) -c hospital.cpp

scheduler: scheduler.cpp network.o
	$(CC) $(CFLAGS) -o scheduler scheduler.cpp network.o

client: client.cpp network.o
	$(CC) $(CFLAGS) -o client client.cpp network.o

network.o: network.cpp network.hpp
	$(CC) $(CFLAGS) -c network.cpp

clean:
	rm -rf *.out *.dSYM *.o hospitalA hospitalB hospitalC scheduler client