build:
	gcc -Wall -Wextra -std=c99 ./src/*.c -lSDL2 -o renderer

run:
	./renderer

clean:
	rm ./renderer