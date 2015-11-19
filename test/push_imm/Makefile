test: push_imm
	objdump -d push_imm | grep -B 1 -A 22 main

push_imm: push_imm.c
	gcc -m32 -o push_imm push_imm.c

clean:
	rm push_imm

