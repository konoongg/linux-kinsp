CLANG    ?= clang
BPFTOOL  ?= bpftool
CC       ?= gcc
CFLAGS   = -g -O2 -Wall
LDFLAGS  = -lbpf -lelf -lz

LIBS_DIR      = ../libs
LIBS_INC      = $(LIBS_DIR)/include
LIBS_LDFLAGS  = -L$(LIBS_DIR) -llk -Wl,-rpath,$(CURDIR)/$(LIBS_DIR)

ARCH     := $(shell uname -m)
BPF_ARCH := $(shell uname -m | sed -e 's/x86_64/x86/')

SOURCES  = src/main.c src/configure.c \
           src/events/filter.c src/events/event.c src/events/profiler.c \
           src/events/event_processor.c
HEADERS  = src/configure.h \
           src/events/filter.h src/events/event.h \
           src/events/profiler.h src/events/event_processor.h

.PHONY: all clean examples

all: main

exec_trace.bpf.o: src/bpf/exec_trace.bpf.c src/events/event.h
	$(CLANG) -target bpf -D__TARGET_ARCH_$(BPF_ARCH) -Isrc -I$(LIBS_INC) -I/usr/include/$(ARCH)-linux-gnu -g -O2 -c $< -o $@

exec_trace.skel.h: exec_trace.bpf.o
	$(BPFTOOL) gen skeleton $< > $@

main: $(SOURCES) $(HEADERS) exec_trace.skel.h
	$(CC) $(CFLAGS) -Isrc -I. -I$(LIBS_INC) -o $@ $(SOURCES) $(LDFLAGS) $(LIBS_LDFLAGS)

examples: examples/hello_exec

examples/hello_exec: examples/hello_exec.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o *.skel.h main examples/hello_exec exectrace.log
