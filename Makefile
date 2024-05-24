CC=gcc
CC_FLAGS=
MAKE=make
SUBDIRS := main parser util
all: $(SUBDIRS)
	$(CC) $(CC_FLAGS) -o bin/fam main/*.o parser/*.o util/*.o
clean:
	rm -rf */*.o
	rm -rf bin/*
test: $(SUBDIRS)
	make -C main test
	make -C parser test
	make -C util test
$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: all $(SUBDIRS)
