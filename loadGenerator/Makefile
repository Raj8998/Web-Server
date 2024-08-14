all: loadgen

loadgen: loadGenerator.c headerInclude.h loadGenThreadRunner.h
	gcc loadGenerator.c -o loadGen

.PHONY: clean

clean:
	rm -rf loadGen