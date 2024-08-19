include Makefile.config

all: release_build
	$(CC) $(RELEASE_FLAGS) $(CC_FLAGS) -o bin/fam $(ALL_OBJS)
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

test_build:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir FLAG_OPTIONS="$(TEST_FLAGS)"; \
	done;

release_build:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir FLAG_OPTIONS="$(RELEASE_FLAGS)"; \
	done;

test: test_build
	ERROR="0"; \
	for dir in $(SUBDIRS); do \
		if test -z $(CRITERION_TEST_PATTERN); then \
			if test -z $(FILTER); then \
				export FILTER=*; \
			fi; \
		        export CRITERION_TEST_PATTERN=$$dir/$$FILTER; \
                fi; \
		if  test -z $(TARGET); then \
			echo "[====] Running $$dir test suite..."; \
			$(MAKE) -C $$dir test FLAG_OPTIONS="$(TEST_FLAGS)" || exit 1; \
		else \
			if [[ "$$dir" == "$(TARGET)" ]]; then \
			        if test -z $(FILTER); then \
					echo "[====] Running $$dir test suite..."; \
				else \
					echo "[====] Running $$dir test suite... (FILTER=$$FILTER)"; \
				fi;\
				$(MAKE) -C $$dir test FLAG_OPTIONS="$(TEST_FLAGS)" || exit 1;\
			fi; \
		fi; \
		if [ $$? -ne "0" ]; then \
			ERROR="1"; \
		fi; \
	done; 
testnc: $(SUBDIRS)
	ERROR="0"; \
	export CRITERION_TEST_PATTERN=$(CRITERION_TEST_PATTERN); \
	for dir in $(SUBDIRS); do \
		if test -z $(CRITERION_TEST_PATTERN); then \
                        if test -z $(FILTER); then \
                                export FILTER=*; \
                        fi; \
                        export CRITERION_TEST_PATTERN=$$dir/$$FILTER; \
                fi; \
		if  test -z $(TARGET); then \
			echo -e "[====] Running $$dir test suite..."; \
			$(MAKE) -C $$dir testnc FLAG_OPTIONS="$(TEST_FLAGS)" || exit 1; \
		else \
			if [[ "$$dir" == "$(TARGET)" ]]; then \
				if test -z $(FILTER); then \
					echo "[====] Running $$dir test suite..."; \
				else \
					echo "[====] Running $$dir test suite... (FILTER=$$FILTER)"; \
				fi;\
				$(MAKE) -C $$dir testnc FLAG_OPTIONS="$(TEST_FLAGS)" || exit 1; \
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

