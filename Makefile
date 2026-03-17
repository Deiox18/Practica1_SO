CC = gcc
CFLAGS = -Wall -g
TARGETS = server ui
OBJS = csv_parser.o

all: $(TARGETS)

server: server.o $(OBJS)
	$(CC) -o $@ $^

ui: ui.o $(OBJS)
	$(CC) -o $@ $^

%.o: %.c common.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o $(TARGETS) /tmp/search_*