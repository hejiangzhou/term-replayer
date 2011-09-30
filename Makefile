TARGETS=record replay
CFLAGS=-O3

.PHONY: all clean

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(TARGETS)
