CC=gcc
CC_FLAGS=
MAKE=make
SUBDIRS := src util parser
all: $(SUBDIRS)
	$(CC) $(CC_FLAGS) -o bin/rustc src/*.o util/*.o parser/*.o
clean:
	rm -rf */*.o
	rm -rf bin/*
$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: all $(SUBDIRS)
