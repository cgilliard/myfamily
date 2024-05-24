include Makefile.config

all: $(SUBDIRS)
	$(CC) $(CC_FLAGS) -o bin/fam $(ALL_OBJS)
clean:
	rm -rf */*.o
	rm -rf bin/*
$(SUBDIRS):
	$(MAKE) -C $@
test: $(SUBDIRS)
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir test; \
	done
.PHONY: all $(SUBDIRS)
.PHONY: test $(SUBDIRS)
