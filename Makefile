include Makefile.config

all: release_build $(HEADERS)
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
    if [ ! -f $(3) ]; then \
        linessum=0; \
        coveredsum=0; \
	rm -f /tmp/gcov_cat.txt; \
        for dir in  $(SUBDIRS); do \
	    echo "=====================================$$dir====================================="; \
	    cd $$dir; \
	    for file in *.c; do \
	       if [ $$file != "test.c" ]; then \
	           echo "Processing: $$dir/$$file"; \
                   gcno_file="$${file%.c}.gcno"; \
		   if [ -f "$$gcno_file" ]; then \
		       percent=`gcov $$file | grep "^Lines" | head -1 | cut -f2 -d ' ' | cut -f2 -d ':' | cut -f1 -d '%' | tr -d \\n`; \
		       if [ "$$percent" == "" ]; then \
		           percent=0.00; \
		       fi; \
                       lines=`gcov $$file | grep "^Lines" | head -1 | cut -f4 -d ' ' | tr -d \\n`; \
		       gcov_file="$$file.gcov"; \
		       if [ -f "$$file.gcov" ]; then \
		           cat $$gcov_file >> /tmp/gcov_cat.txt; \
		           if [ "$$lines" == "" ]; then \
                               lines=0; \
                           fi; \
		           ratio=`awk "BEGIN {print $$percent / 100}"`; \
		           covered=`awk "BEGIN {print int($$ratio * $$lines)}"`; \
		           linessum=`awk "BEGIN {print $$linessum + $$lines}"`; \
                           coveredsum=`awk "BEGIN {print $$coveredsum + $$covered}"`; \
		           echo "-->Results for $$file. percent=$$percent%. lines=$$lines. ratio=$$ratio. covered=$$covered."; \
		       else \
                           echo "-->No gcov file for $$file. Not including in results."; \
		       fi; \
		   else \
		       echo "-->No gcno file for $$file. Not including in results."; \
		   fi; \
		   echo "------------------------------------------------------------------------------"; \
               fi; \
	    done; \
	    cd ..; \
	done; \
        codecov=`awk "BEGIN {print 100 * $$coveredsum / $$linessum}"` \
        codecov=`printf "%.2f" $$codecov`; \
	echo "$$codecov" > /tmp/cc_final; \
	echo "===================================SUMMARY===================================="; \
	timestamp=$$(date +%s); \
        echo "CodeCoverage=$$codecov%, CoveredLines=($$coveredsum of $$linessum)."; \
	echo "$$timestamp $$codecov $$coveredsum $$linessum" > /tmp/codecov; \
    fi; \
    exit $$ERROR
endef

test_build:
	$(CC) -o bin/xxdir build_utils/xxdir.c
	bin/xxdir main/resources main/resources.h fam
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir FLAG_OPTIONS="$(TEST_FLAGS)"; \
	done;

release_build:
	$(CC) -o bin/xxdir build_utils/xxdir.c
	bin/xxdir main/resources main/resources.h fam
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir FLAG_OPTIONS="$(RELEASE_FLAGS)"; \
	done;

san_build:
	$(CC) -o bin/xxdir build_utils/xxdir.c
	bin/xxdir main/resources main/resources.h fam
	 for dir in $(SUBDIRS); do \
		 $(MAKE) -C $$dir FLAG_OPTIONS="$(SAN_FLAGS)"; \
	done;

coverage_build:
	$(CC) -o bin/xxdir build_utils/xxdir.c
	bin/xxdir main/resources main/resources.h fam
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir FLAG_OPTIONS="$(COVERAGE_FLAGS)"; \
	done; \

test: test_build
	 $(call run_tests,$(TEST_FLAGS),test)

coverage: coverage_build
	$(call run_tests,$(COVERAGE_FLAGS),test,coverage);

testsan: san_build
	$(call run_tests,$(SAN_FLAGS),testnc)

testnc: test_build
	$(call run_tests,$(TEST_FLAGS),testnc)

.PHONY: doc
