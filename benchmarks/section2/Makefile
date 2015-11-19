CC=cc 

# settings depend on whether we have binary translation enabled
ifeq ($(use_bt),true)
CFLAGS = -O2 -I../utils -DENABLE_BT
LIBS = -lm -L../utils -lgetclock -lfastbt -lpthread -ldl
resultfile = results_bt.csv
else
CFLAGS = -O2 -I../utils 
LIBS = -lm -L../utils -lgetclock
resultfile = results_native.csv
endif



tempresult = currentresult.temp
fulloutput = ../full_output

CLOCKFLAG = -DCGT 
LDFLAGS = 

prettysequence = FFTBenchSizeA FFTBenchSizeB FFTBenchSizeC HeapSortBenchSizeA HeapSortBenchSizeB HeapSortBenchSizeC LUFactBenchSizeA LUFactBenchSizeB LUFactBenchSizeC SeriesBenchSizeA SeriesBenchSizeB SeriesBenchSizeC SORBenchSizeA SORBenchSizeB SORBenchSizeC SparseMatmultBenchSizeA SparseMatmultBenchSizeB SparseMatmultBenchSizeC


SIZEA = FFTBenchSizeA HeapSortBenchSizeA LUFactBenchSizeA SeriesBenchSizeA SORBenchSizeA SparseMatmultBenchSizeA
SIZEB = FFTBenchSizeB HeapSortBenchSizeB LUFactBenchSizeB SeriesBenchSizeB SORBenchSizeB SparseMatmultBenchSizeB
SIZEC = FFTBenchSizeC HeapSortBenchSizeC LUFactBenchSizeC SeriesBenchSizeC SORBenchSizeC SparseMatmultBenchSizeC
ALL = ${SIZEA} ${SIZEB} ${SIZEC} 

ifeq ($(run_all),true)
run_benchmarks = $(prettysequence)
else
run_benchmarks = $(SIZEA)
endif


.c.o:
	${CC} ${CFLAGS}  -c $*.c 

default:${SIZEA} 

all: ${ALL} 

getclock.o: getclock.c 
	$(CC) $(CFLAGS) $(CLOCKFLAG) -c $*.c

LUFactBenchSizeA: LUFactSizeA.o LUFact.o  
	$(CC) -o LUFactBenchSizeA LUFactSizeA.o LUFact.o ${LIBS} 

LUFactBenchSizeB: LUFactSizeB.o LUFact.o  
	$(CC) -o LUFactBenchSizeB LUFactSizeB.o LUFact.o ${LIBS} 

LUFactBenchSizeC: LUFactSizeC.o LUFact.o  
	$(CC) -o LUFactBenchSizeC LUFactSizeC.o LUFact.o ${LIBS} 

 
HeapSortBenchSizeA: HeapSortSizeA.o HeapSort.o  
	$(CC) -o HeapSortBenchSizeA HeapSortSizeA.o HeapSort.o ${LIBS} 

HeapSortBenchSizeB: HeapSortSizeB.o HeapSort.o  
	$(CC) -o HeapSortBenchSizeB HeapSortSizeB.o HeapSort.o ${LIBS} 

HeapSortBenchSizeC: HeapSortSizeC.o HeapSort.o  
	$(CC) -o HeapSortBenchSizeC HeapSortSizeC.o HeapSort.o ${LIBS} 

CryptBenchSizeA: CryptSizeA.o Crypt.o  
	$(CC) -o CryptBenchSizeA CryptSizeA.o Crypt.o ${LIBS} 

SORBenchSizeA: SORSizeA.o SOR.o  
	$(CC) -o SORBenchSizeA SORSizeA.o SOR.o ${LIBS} 

SORBenchSizeB: SORSizeB.o SOR.o  
	$(CC) -o SORBenchSizeB SORSizeB.o SOR.o ${LIBS} 

SORBenchSizeC: SORSizeC.o SOR.o  
	$(CC) -o SORBenchSizeC SORSizeC.o SOR.o ${LIBS} 

FFTBenchSizeA: FFTSizeA.o FFT.o  
	$(CC) -o FFTBenchSizeA FFTSizeA.o FFT.o ${LIBS} 

FFTBenchSizeB: FFTSizeB.o FFT.o  
	$(CC) -o FFTBenchSizeB FFTSizeB.o FFT.o ${LIBS} 

FFTBenchSizeC: FFTSizeC.o FFT.o  
	$(CC) -o FFTBenchSizeC FFTSizeC.o FFT.o ${LIBS} 

SparseMatmultBenchSizeA: SparseMatmultSizeA.o SparseMatmult.o  
	$(CC) -o SparseMatmultBenchSizeA SparseMatmultSizeA.o SparseMatmult.o ${LIBS} 

SparseMatmultBenchSizeB: SparseMatmultSizeB.o SparseMatmult.o  
	$(CC) -o SparseMatmultBenchSizeB SparseMatmultSizeB.o SparseMatmult.o ${LIBS} 

SparseMatmultBenchSizeC: SparseMatmultSizeC.o SparseMatmult.o  
	$(CC) -o SparseMatmultBenchSizeC SparseMatmultSizeC.o SparseMatmult.o ${LIBS} 

SeriesBenchSizeA: SeriesSizeA.o Series.o  
	$(CC) -o SeriesBenchSizeA SeriesSizeA.o Series.o ${LIBS} 

SeriesBenchSizeB: SeriesSizeB.o Series.o  
	$(CC) -o SeriesBenchSizeB SeriesSizeB.o Series.o ${LIBS} 

SeriesBenchSizeC: SeriesSizeC.o Series.o  
	$(CC) -o SeriesBenchSizeC SeriesSizeC.o Series.o ${LIBS} 

cleanresults:
	rm -f results.csv results_bt.csv results_native.csv results_precise.csv
	rm -f $(tempresult)
	rm -f $(fulloutput)

# TODO: replace SIZEA by prettysequence
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
	echo "benchmark name,time1,time2,time3,time4,time5" > $(resultfile)
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

.IGNORE:

clean: 
	rm -f *.o ${ALL} 




