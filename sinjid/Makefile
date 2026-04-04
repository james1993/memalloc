CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -Wno-missing-field-initializers -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lraylib -lm -lpthread -ldl

SRCS    = main.c player.c combat.c data.c hub.c ui.c
OBJS    = $(SRCS:.c=.o)
TARGET  = sinjid

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: all
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
