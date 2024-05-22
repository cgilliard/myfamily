CC=gcc
CC_FLAGS=
MAKE=make
SUBDIRS := main parser
all: $(SUBDIRS)
	$(CC) $(CC_FLAGS) -o bin/rustc main/*.o parser/*.o
clean:
	rm -rf */*.o
	rm -rf bin/*
$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: all $(SUBDIRS)
