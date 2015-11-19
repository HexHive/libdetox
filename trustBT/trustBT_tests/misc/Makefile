
test: test.c
	gcc -O2  -ggdb -Wall -DDEBUG -DDUMP_GENERATED_CODE -Winline -finline-limit=30000 --param inline-unit-growth=512 --param large-function-growth=2048 -funit-at-a-time -o test test.c -lfastbt -lpthread -lm -ldl

clean:
	rm -f test
	rm -f *.o *~ jmpTable_dump.txt code_dump.txt debug.txt


