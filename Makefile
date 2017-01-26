all:
	gcc -c test.c
	gcc -o main test.o /usr/lib/libzbc.so -lrt
test:
	gcc -c writesmr.c
	gcc -o main writesmr.o -lrt
clean:
	rm -rf main
	rm -rf *.o
