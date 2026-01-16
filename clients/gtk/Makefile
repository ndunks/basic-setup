# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 `pkg-config --cflags gtk4`
LDFLAGS =  -lwebsockets `pkg-config --libs gtk4`

# Target
TARGET = shb

# Source files
SRCS = shb.c websocket_client.c

# Object files
OBJS = $(SRCS:.c=.o)

# Rules
.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)