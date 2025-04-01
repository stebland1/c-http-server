CC = gcc

CFLAGS = -Wall -I./include

SRC = src/main.c src/utils.c src/request_handler.c src/str_builder.c src/dir_list.c

OBJ = $(SRC:.c=.o)

TARGET = build/http-server

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
