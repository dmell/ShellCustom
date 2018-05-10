#!/bin/bash
make
if [[ $? -eq 0 ]] ; then
	cd bin/
	./shell -o="out.txt" -e="err.txt" -c
fi
