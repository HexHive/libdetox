CC=cc 

# settings depend on whether we have binary translation enabled
ifeq ($(use_bt),true)
CFLAGS = -O2 -I../utils -DENABLE_BT
LIBS = -lm -L../utils -lgetclock -lfastbt -lpthread  -ldl
resultfile = results_bt.csv
else
CFLAGS = -O2 -I../utils 
LIBS = -lm -L../utils -lgetclock
resultfile = results_native.csv
endif


tempresult = currentresult.temp
fulloutput = ../full_output


LDFLAGS = 


prettysequence = EulerBenchSizeA EulerBenchSizeB MolDynBenchSizeA MolDynBenchSizeB

SIZEA = EulerBenchSizeA MolDynBenchSizeA
SIZEB = EulerBenchSizeB MolDynBenchSizeB

ALL = ${SIZEA} ${SIZEB} 

ifeq ($(run_all),true)
run_benchmarks = $(prettysequence)
else
run_benchmarks = $(SIZEA)
endif




.c.o:
	${CC} ${CFLAGS}  -c $*.c 

default:${SIZEA} 

all: ${ALL} 

EulerBenchSizeA: EulerSizeA.o Euler.o  
	$(CC) -o EulerBenchSizeA EulerSizeA.o Euler.o ${LIBS} 

EulerBenchSizeB: EulerSizeB.o Euler.o ${CLOCKOBJS} 
	$(CC) -o EulerBenchSizeB EulerSizeB.o Euler.o ${LIBS} 

MolDynBenchSizeA: MolDynSizeA.o MolDyn.o  
	$(CC) -o MolDynBenchSizeA MolDynSizeA.o MolDyn.o ${LIBS} 

MolDynBenchSizeB: MolDynSizeB.o MolDyn.o ${CLOCKOBJS} 
	$(CC) -o MolDynBenchSizeB MolDynSizeB.o MolDyn.o ${LIBS} 

clean: 
	rm -f *.o ${ALL} 

cleanresults:
	rm -f results.csv results_bt.csv results_native.csv results_precise.csv
	rm -f $(tempresult)
	rm -f $(fulloutput)


.IGNORE:


run_benchmark:	.IGNORE
ifneq ($(use_bt), true)
	echo "+++ output for all tests *without* binary translation +++" >> $(fulloutput)
	echo "benchmark name,success without bt,time without bt" > $(resultfile)
	for file in $(run_benchmarks); do \
		./$${file} > $(tempresult); \
		echo -n "$${file}," >> $(resultfile); \
		grep failed $(tempresult); \
		if [ $${?} -eq "0" ]; then \
			echo -n "validation failed" >> $(resultfile); \
		fi; \
		echo -n "," >> $(resultfile); \
		cat $(tempresult) | tail -n 1 | sed 's/..*[[:space:]]*[^[:digit:]]\([[:digit:]]*[.][[:digit:]]*\)[[:space:]]*(s).*/\1/' >> $(resultfile); \
		cat $(tempresult) >> $(fulloutput); \
	done
else
	echo "+++ output for all tests *with* binary translation +++" >> $(fulloutput)
	echo "success with bt,time with bt" > $(resultfile)
	for file in $(run_benchmarks); do \
		./$${file} > $(tempresult); \
		grep failed $(tempresult); \
		if [ $${?} -eq "0" ]; then \
			echo -n "validation failed" >> $(resultfile); \
		fi; \
		echo -n "," >> $(resultfile); \
		cat $(tempresult) | tail -n 1 | sed 's/..*[[:space:]]*[^[:digit:]]\([[:digit:]]*[.][[:digit:]]*\)[[:space:]]*(s).*/\1/' >> $(resultfile); \
		cat $(tempresult) >> $(fulloutput); \
	done
	paste -d , results_native.csv results_bt.csv > results.csv
endif

precise_benchmarks: resultfile = results_precise.csv
precise_benchmarks: .IGNORE
	for file in $(run_benchmarks); do \
		echo -n "$${file}" >> $(resultfile); \
		for i in 1 2 3 4 5; do \
			echo "$${file} run $${i} of 5"; \
			./$${file} > $(tempresult); \
			echo -n "," >> $(resultfile); \
			cat $(tempresult) | tail -n 1 | sed 's/..*[[:space:]]*[^[:digit:]]\([[:digit:]]*[.][[:digit:]]*\)[[:space:]]*(s).*/\1/' | tr -d '\n' >> $(resultfile); \
			cat $(tempresult) >> $(fulloutput); \
		done; \
		echo >> $(resultfile); \
	done



