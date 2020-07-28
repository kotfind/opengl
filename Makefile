CFLAGS=-lGL -lSDL2

all: a.out

a.out: main.o
	g++ main.o $(CFLAGS)

main.o: main.cpp
	g++ -c main.cpp

run: a.out
	./a.out

clean:
	rm -f a.out
	rm -f main.o
