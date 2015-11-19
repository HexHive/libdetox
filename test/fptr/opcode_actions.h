typedef struct INSTR {
	char mnemonic[8];
	void (*action)(int);	/* function pointer */
} instr;


void default_action(int pos);

void do_something_fancy(int pos);

void do_something_cool(int pos);
