CC = gcc

CFLAGS = -Wall -I./include

SRC = src/main.c src/utils.c

OBJ = $(SRC:.c=.o)

TARGET = build/http-server

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
