CC=gcc
CC_FLAGS=
MAKE=make
SUBDIRS := main parser util
all: $(SUBDIRS)
	$(CC) $(CC_FLAGS) -o bin/fam main/*.o parser/*.o util/*.o
clean:
	rm -rf */*.o
	rm -rf bin/*
$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: all $(SUBDIRS)
