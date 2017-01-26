all:
	gcc -c writesmr.c
	gcc -o main writesmr.o -lrt
clean:
	rm -rf main
	rm -rf *.o
