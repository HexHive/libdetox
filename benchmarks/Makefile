# allows building benchmarks without installing
CURRENT_DIR:=$(shell pwd)
export CPATH = $(CURRENT_DIR)/../include
export LIBRARY_PATH = $(CURRENT_DIR)/../lib

all: utils nativebench btbench graph

.PHONY: utils clean cleanbench cleanresults

utils:
	$(MAKE) -C utils

clean:
	$(MAKE) cleanbench
	$(MAKE) cleanresults
	$(MAKE) -C utils clean

# clean test bench (all compiled files will be cleaned)
cleanbench:
	$(MAKE) -C section2 clean
	$(MAKE) -C section3 clean

# remove result files from previous benchmark runs
cleanresults:
	$(MAKE) -C section2 cleanresults
	$(MAKE) -C section3 cleanresults
	rm -f results.csv
	rm -f results.xhtml
	rm -f results_precise.csv


# run tests without binary translation
######################################

# target-specific variable
nativebench:	export use_bt=false
nativebench:
	$(MAKE) cleanresults
	$(MAKE) cleanbench
	$(MAKE) -C section2 all
	$(MAKE) -C section3 all
	$(MAKE) -C section2 run_benchmark
	$(MAKE) -C section3 run_benchmark


# run tests with binary translation enabled
###########################################

# target-specific variable
btbench:	export use_bt=true
btbench:
	$(MAKE) cleanbench
	$(MAKE) -C section2 all
	$(MAKE) -C section3 all
	$(MAKE) -C section2 run_benchmark
	$(MAKE) -C section3 run_benchmark

precise_benchmarks: export run_all=true
precise_benchmarks:
	$(MAKE) cleanresults
	$(MAKE) -C section2 precise_benchmarks
	$(MAKE) -C section3 precise_benchmarks
	cat section2/results_precise.csv > results_precise.csv
	cat section3/results_precise.csv >> results_precise.csv


# make a html-file with embedded svg for graphs
graph:
	cat section2/results.csv > results.csv
	cat section3/results.csv >> results.csv
	./drawgraph.pl > results.xhtml
