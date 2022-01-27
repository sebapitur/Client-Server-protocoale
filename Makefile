CFLAGS = -Wall -g

all: server subscriber

server: server.c server_util.c LinkedList.c

subscriber: subscriber.c subscriber_util.c

.PHONY: clean

clean:
	rm server subscriber
