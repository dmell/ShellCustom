.PHONY: build clean

build: temp | shell

clean:
	@rm -rf ./bin
	@mv src/shell.c .
	@mv src/shellutil.c .
	@mv src/shellutil.h .
	@rm -rf ./src

temp:
	@echo "Generating subfolders and temporary files..."
	@mkdir bin
	@mkdir src
	@mkdir ./src/tmp

shell:
	@echo "Shell is compiling..."
	@gcc -o shell shellutil.c shell.c
	@mv shell.c ./src
	@mv shellutil.c ./src
	@mv shellutil.h ./src
	@mv shell ./bin
