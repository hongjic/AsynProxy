.PHONY: clean

tcpproxy: sockutils.o connection.o cirbuffer.o tcpproxy.c
	gcc -c tcpproxy.c
	gcc -o tcpproxy tcpproxy.o sockutils.o cirbuffer.o connection.o

clean:
	rm *.o; rm dumbfetch; rm dumbfetch2;

sockutils.o: sockutils.c
	gcc -c sockutils.c

cirbuffer.o: cirbuffer.c
	gcc -c cirbuffer.c

connection.o: cirbuffer.o sockutils.o connection.c
	gcc -c connection.c