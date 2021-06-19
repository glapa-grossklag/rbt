SRC    = $(wildcard *.c)
OBJ    = $(SRC:%.c=%.o)
BIN    = rb-test

CC     = clang
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

format:
	find . -name "*.[ch]" | xargs clang-format -i -style=file
