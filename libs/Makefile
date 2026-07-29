CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -fPIC -Iinclude
TARGET  = liblk.so

SRCS = src/allocator/lk_allocator.c \
       src/allocator/lk_libc_alloc.c \
       src/collections/lk_memblock.c \
       src/collections/lk_string.c \
       src/object/lk_obj.c \
       src/object/lk_status.c \
       src/files/lk_file.c \
       src/logger/lk_logger.c \
       src/logger/lk_std_logger.c \
       src/logger/lk_determ_logger.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -shared -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
