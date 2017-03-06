CC=g++

main: common.hpp dp_heuristic.hpp main.cpp
	$(CC) -std=c++11 -lpthread -O3 -Wall -pedantic -fmax-errors=1 -o main main.cpp

debug: common.hpp dp_heuristic.hpp main.cpp
	$(CC) -D_GLIBCXX_DEBUG -DDEBUG -std=c++11 -lpthread -g -Wall -pedantic -fmax-errors=1 -o main main.cpp
	
