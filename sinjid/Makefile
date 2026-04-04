CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -Wno-missing-field-initializers \
          -Wno-stringop-truncation \
          -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lraylib -lm -lpthread -ldl

UI_SRCS = ui/ui_common.c \
          ui/ui_title.c \
          ui/ui_classselect.c \
          ui/ui_hub.c \
          ui/ui_combat.c \
          ui/ui_inventory.c \
          ui/ui_shop.c \
          ui/ui_skills.c \
          ui/ui_levelup.c \
          ui/ui_gateway.c \
          ui/ui_gameover.c \
          ui/ui_victory.c

SRCS    = main.c player.c combat.c data.c data_loader.c hub.c \
          audio.c anim.c save.c scene.c events.c config.c update.c \
          $(UI_SRCS)

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
