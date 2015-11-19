/* 
 * Test alternate calling convention "stdcall", where the callee releases the space
 * that the arguments occupied on the stack. This is done by using an immediate as 
 * argument to the "ret" instruction.
 */

int add1(int op1, int op2)	__attribute__((stdcall));

int add2(int op1, int op2);

int main(int argc, char **argv)
{
	int i = add1(3, 5);
}

int __attribute__((stdcall)) add1(int op1, int op2)
{
	
	return add2(op1, op2);
}

/* second function to see if the calling convention is propagated... */
int add2(int op1, int op2)
{
	return op1 + op2;
}

