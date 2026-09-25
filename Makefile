.PHONY: all libs exectrace examples clean

all: libs exectrace examples

libs:
	$(MAKE) -C libs

exectrace: libs
	$(MAKE) -C exectrace

examples:
	$(MAKE) -C exectrace examples

clean:
	$(MAKE) -C libs clean
	$(MAKE) -C exectrace clean