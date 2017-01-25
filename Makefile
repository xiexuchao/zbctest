All:
	gcc -c test.c
	gcc -o main test.o /usr/lib/libzbc.so
clean:
	rm *.o main
