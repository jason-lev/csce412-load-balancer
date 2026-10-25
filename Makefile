CC = g++
CFLAGS = -Wall -Werror

all: load-balancer
load-balancer: main.o utils.o firewall.o
	$(CC) $(CFLAGS) -o load-balancer main.o utils.o firewall.o

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

utils.o: utils.cpp
	$(CC) $(CFLAGS) -c utils.cpp

firewall.o: firewall.cpp
	$(CC) $(CFLAGS) -c firewall.cpp

clean:
	rm -f load-balancer *.o