CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -Iinclude
TARGET  = banca_core

SRCS = src/main.c \
       src/utenti.c \
       src/conti.c \
       src/transazioni.c \
       src/file_io.c \
       src/crypto.c \
       src/utils.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lcrypto

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
