All:
	gcc -c hasmr.c
	gcc -o main hasmr.o /usr/lib/libzbc.so
clean:
	rm *.o main
