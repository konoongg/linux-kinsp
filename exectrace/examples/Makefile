CC      ?= gcc
CFLAGS  ?= -g -O2 -Wall

SRCS    := $(wildcard *.c)
EXAMPLES = $(SRCS:.c=)

.PHONY: all clean

all: $(EXAMPLES)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(EXAMPLES) *.log