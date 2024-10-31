CC = gcc
CFLAGS = -Wall

SRC = tkshell.c
OBJ = tkshell.o

TARGET = tkshell


all: $(TARGET)

compile: $(OBJ)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

clean:
	rm -f $(OBJ) $(TARGET)

