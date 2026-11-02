CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = pos
OBJ = main.o data.o

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

main.o: main.c pos.h
data.o: data.c pos.h

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET)
