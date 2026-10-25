CC = g++
CFLAGS = -Wall -Werror

all: load-balancer
load-balancer: main.o utils.o
	$(CC) $(CFLAGS) -o load-balancer main.o utils.o

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

utils.o: utils.cpp
	$(CC) $(CFLAGS) -c utils.cpp

clean:
	rm -f load-balancer *.o