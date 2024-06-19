include Makefile.config

all: $(SUBDIRS)
	$(CC) $(CC_FLAGS) -o bin/fam $(ALL_OBJS)
clean:
	rm -rf */*.o
	rm -rf bin/*
	rm -rf */*.gcno
	rm -rf */*.gcda
	rm -rf */*.gcov
	rm -rf *.gcno
	rm -rf *.gcda
	rm -rf *.gcov
$(SUBDIRS):
	$(MAKE) -C $@
test: $(SUBDIRS)
	ERROR="0"; \
	for dir in $(SUBDIRS); do \
		if test -z $(CRITERION_TEST_PATTERN); then \
                        CRITERION_TEST_PATTERN=$$dir/*; \
                        export CRITERION_TEST_PATTERN; \
                fi; \
		if  test -z $(TARGET); then \
			$(MAKE) -C $$dir test; \
		else \
			if [[ "$$dir" == "$(TARGET)" ]]; then \
				$(MAKE) -C $$dir test; \
			fi; \
		fi; \
		if [ $$? -ne "0" ]; then \
			ERROR="1"; \
		fi; \
	done; 
testnc: $(SUBDIRS)
	ERROR="0"; \
	for dir in $(SUBDIRS); do \
		if test -z $(CRITERION_TEST_PATTERN); then \
			CRITERION_TEST_PATTERN=$$dir/*; \
			export CRITERION_TEST_PATTERN; \
		fi; \
		if  test -z $(TARGET); then \
		$(MAKE) -C $$dir testnc; \
		else \
			if [[ "$$dir" == "$(TARGET)" ]]; then \
				$(MAKE) -C $$dir testnc; \
			fi; \
		fi; \
		if [ $$? -ne "0" ]; then \
		ERROR="1"; \
		fi; \
	done;
doc:
	doxygen doxygen.cfg
install:
	cp bin/fam $(INSTALL_DIR)
.PHONY: all $(SUBDIRS)
.PHONY: test $(SUBDIRS)
.PHONY: doc

