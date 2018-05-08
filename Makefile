.PHONY: build clean

build: temp | shell

clean:
	@rm -rf ./bin
	@mv src/shell.c .
	@rm -rf ./src

temp:
	@echo "Generating subfolders and temporary files..."
	@mkdir bin
	@mkdir src
	@mkdir ./src/tmp

shell:
	@echo "Shell is compiling..."
	@gcc -o shell shell.c
	@mv shell.c ./src
	@mv shell ./bin
