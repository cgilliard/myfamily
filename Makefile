include Makefile.config

all: release_build
	$(CC) $(RELEASE_FLAGS) $(CC_FLAGS) -o bin/fam $(ALL_OBJS)

doc:
	doxygen doxygen.cfg

install:
	cp bin/fam $(INSTALL_DIR)

clean:
	rm -rf */*.o
	rm -rf bin/*
	rm -rf */*.gcno
	rm -rf */*.gcda
	rm -rf */*.gcov
	rm -rf *.gcno
	rm -rf *.gcda
	rm -rf *.gcov

define run_tests
    ERROR="0"; \
    for dir in $(SUBDIRS); do \
        if test -z $(CRITERION_TEST_PATTERN); then \
            if test -z $(FILTER); then \
                export FILTER=*; \
            fi; \
            export CRITERION_TEST_PATTERN=$$dir/$$FILTER; \
        fi; \
        if test -z $(TARGET); then \
            echo "[====] Running $$dir test suite..."; \
            $(MAKE) -C $$dir $(2) FLAG_OPTIONS="$(1)" || exit 1; \
        else \
            if [[ "$$dir" == "$(TARGET)" ]]; then \
                if test -z $(FILTER); then \
                    echo "[====] Running $$dir test suite..."; \
                else \
                    echo "[====] Running $$dir test suite... (FILTER=$$FILTER)"; \
                fi; \
                $(MAKE) -C $$dir $(2) FLAG_OPTIONS="$(1)" || exit 1; \
            fi; \
        fi; \
        if [ $$? -ne "0" ]; then \
            ERROR="1"; \
        fi; \
    done; \
    exit $$ERROR
endef

test_build:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir FLAG_OPTIONS="$(TEST_FLAGS)"; \
	done;

release_build:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir FLAG_OPTIONS="$(RELEASE_FLAGS)"; \
	done;

san_build:
	 for dir in $(SUBDIRS); do \
		 $(MAKE) -C $$dir FLAG_OPTIONS="$(SAN_FLAGS)"; \
	done;

coverage_build:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir FLAG_OPTIONS="$(COVERAGE_FLAGS)"; \
	done;

test: test_build
	 $(call run_tests,$(TEST_FLAGS),test)

coverage: coverage_build
	$(call run_tests,$(COVERAGE_FLAGS),test)

testsan: san_build
	$(call run_tests,$(SAN_FLAGS),test)

testnc: test_build
	$(call run_tests,$(TEST_FLAGS),testnc)

.PHONY: doc
