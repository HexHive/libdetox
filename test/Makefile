all: inst_pointer_read inst_pointer_modify

inst_pointer_read:	inst_pointer_read.c
	gcc -m32 -o inst_pointer_read inst_pointer_read.c

inst_pointer_modify:	inst_pointer_modify.c
	gcc -m32 -o inst_pointer_modify inst_pointer_modify.c

.PHONY: clean

clean: 
	rm inst_pointer_read inst_pointer_modify

