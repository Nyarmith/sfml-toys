LIBS=-lsfml-graphics -lsfml-window -lsfml-system
CC=g++
OPTS=-std=c++14 -O2 -g

particles:
	${CC} ${OPTS} particles.cpp -o particles.exe ${LIBS}

all:
	particles

