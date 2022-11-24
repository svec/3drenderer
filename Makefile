build:
	gcc -Wall -Wextra -std=c99 ./src/*.c -lSDL2 -lm -o renderer -lM

run:
	./renderer

clean:
	rm ./renderer