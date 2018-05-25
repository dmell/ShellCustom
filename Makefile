.PHONY: build clean

build: folders | shell

clean:
	@rm -rf ./bin

folders:
	@echo "Generating subfolders and temporary files..."
	@mkdir bin

shell:
	@echo "Shell is compiling..."
	@gcc -std=gnu90 -o shell ./src/shellutil.c ./src/parsers.c ./src/run.c ./src/commons.c ./src/shell.c
	@mv shell ./bin
