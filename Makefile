.PHONY: build clean

build: shell
	@echo "Shell is compiling..."

clean:
	@rm -rf ./bin
	mv src/shell_01.c .
	@rm -rf ./src

shell:
	mkdir bin
	mkdir src
	gcc -o shell shell_01.c
	mv shell_01.c ./src
	mv shell ./bin
