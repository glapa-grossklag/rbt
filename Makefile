SRC    = $(wildcard *.c)
OBJ    = $(SRC:%.c=%.o)
BIN    = rb-test

CC     = gcc
CFLAGS = -Wall -Wextra -O0

.PHONY: all tidy clean debug format

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

tidy:
	rm -f $(OBJ)

clean: tidy
	rm -f $(BIN)

debug: CFLAGS += -O0 -g
debug: all

gdb: debug
	gdb -se $(BIN) -ex run -ex backtrace -q

format:
	find . -name "*.[ch]" | xargs clang-format -i -style=file
