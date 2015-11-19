
fptr: fptr.o opcode_actions.o
	gcc -m32 -O2 -o fptr fptr.o opcode_actions.o


fptr.o: fptr.c table.map opcode_actions.h
	gcc -m32 -c -O2 -o fptr.o fptr.c

opcode_actions.o: opcode_actions.c opcode_actions.h
	gcc -m32 -c -O2 -o opcode_actions.o opcode_actions.c


clean:	
	rm fptr
	rm fptr.o
	rm opcode_actions.o
