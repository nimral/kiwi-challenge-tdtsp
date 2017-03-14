CC=g++

all: debug main

debug: common.hpp dp_heuristic.hpp random_perturbations.hpp main.cpp
	$(CC) -D_GLIBCXX_DEBUG -DDEBUG -std=c++11 -lpthread -fopenmp -g -Wall -pedantic -fmax-errors=1 -o debug main.cpp
	
prof: common.hpp dp_heuristic.hpp random_perturbations.hpp main.cpp
	$(CC) -std=c++11 -lpthread -fopenmp -O2 -Wall -pedantic -pg -fmax-errors=1 -o prof main.cpp

main: common.hpp dp_heuristic.hpp random_perturbations.hpp main.cpp
	$(CC) -std=c++11 -lpthread -fopenmp -O3 -Wall -pedantic -fmax-errors=1 -o main main.cpp

