CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -pedantic -g
TARGET  = rtos_scheduler
SRCS    = main.c rtos.c
OBJS    = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c rtos.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
