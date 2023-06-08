SDL_ROOT_DIR = /opt/homebrew
SDL_INC_DIR = ${SDL_ROOT_DIR}/include
SDL_LIB_DIR = ${SDL_ROOT_DIR}/lib

CFLAGS=-I${SDL_INC_DIR} -D_THREAD_SAFE
CFLAGS += -g -Wall -Wextra -std=c99

LFLAGS= -L${SDL_LIB_DIR} -lSDL2 -lm -lM

build:
	gcc ${CFLAGS} ./src/*.c ${LFLAGS} -o renderer

run:
	./renderer

clean:
	rm ./renderer