include Makefile.config

all: $(SUBDIRS)
	$(CC) $(CC_FLAGS) -o bin/fam $(ALL_OBJS)
clean:
	rm -rf */*.o
	rm -rf bin/*
$(SUBDIRS):
	$(MAKE) -C $@
test: $(SUBDIRS)
	ERROR="0"; \
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir test; \
		if [ $$? -ne "0" ]; then \
		ERROR="1"; \
		echo "error!"; \
		fi; \
	done; \
	if [ $$ERROR -ne "0" ]; then \
		$$? = 1; \
	fi
.PHONY: all $(SUBDIRS)
.PHONY: test $(SUBDIRS)
