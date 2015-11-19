# do not invoke make in this directory! always invoke make in the libfastbt directory

#LDFLAGS = -lfastbt -lpthread -static-libgcc
LDFLAGS = -lfastbt -lpthread -lm -ldl

demoprog: simpletest longertest mediumtest functiontest adv_functiontest jcctest ind_jmp_test ind_jmp_reg_test printftest bintree pretranslated_functiontest floattest chainingtest ret_predict_test mathtest powtest

simpletest: simpletest.c
	gcc $(CFLAGS) -o simpletest simpletest.c $(LDFLAGS)

longertest: longertest.c
	gcc $(CFLAGS) -o longertest longertest.c $(LDFLAGS)

mediumtest: mediumtest.c
	gcc $(CFLAGS) -o mediumtest mediumtest.c $(LDFLAGS)
	
floattest: floattest.c
	gcc $(CFLAGS) -o floattest floattest.c $(LDFLAGS)

functiontest: functiontest.c
	gcc $(CFLAGS) -o functiontest functiontest.c $(LDFLAGS)

adv_functiontest: adv_functiontest.c
	gcc $(CFLAGS) -o adv_functiontest adv_functiontest.c $(LDFLAGS)

jcctest: jcctest.c
	gcc $(CFLAGS) -o jcctest jcctest.c $(LDFLAGS)

ind_jmp_test: ind_jmp_test.S
	gcc $(CFLAGS) -o ind_jmp_test ind_jmp_test.S $(LDFLAGS)

ind_jmp_reg_test: ind_jmp_reg_test.S
	gcc $(CFLAGS) -o ind_jmp_reg_test ind_jmp_reg_test.S $(LDFLAGS)

printftest: printftest.c
	gcc $(CFLAGS) -o printftest printftest.c $(LDFLAGS)

bintree: bintree.c
	gcc $(CFLAGS) -o bintree bintree.c $(LDFLAGS)

pretranslated_functiontest: pretranslated_functiontest.c
	gcc $(CFLAGS) -o pretranslated_functiontest pretranslated_functiontest.c $(LDFLAGS)

chainingtest: chainingtest.c
	gcc $(CFLAGS) -o chainingtest chainingtest.c $(LDFLAGS)

ret_predict_test: ret_predict_test.S
	gcc $(CFLAGS) -o ret_predict_test ret_predict_test.S $(LDFLAGS)

mathtest: mathtest.c
	gcc $(CFLAGS) -O4 -g -o mathtest mathtest.c $(LDFLAGS)

powtest: powtest.c
	gcc $(CFLAGS) -o powtest powtest.c $(LDFLAGS)

clean:
	rm -f simpletest longertest mediumtest functiontest adv_functiontest jcctest ind_jmp_test ind_jmp_reg_test printftest bintree pretranslated_functiontest floattest chainingtest ret_predict_test mathtest powtest
	rm -f *.o *~ jmpTable_dump.txt code_dump.txt debug.txt


