CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=c11 -I/usr/local/include
LDFLAGS = -L/usr/local/lib
LIBS    = -lraylib -lm -lpthread -ldl -lX11

TARGET  = game
SRC     = game.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: clean
