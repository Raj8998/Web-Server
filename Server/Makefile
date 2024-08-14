all: server

server: serverApplication.c headerInclude.h threadRunner.h
	gcc serverApplication.c -o server

.PHONY: clean

clean:
	rm -rf server