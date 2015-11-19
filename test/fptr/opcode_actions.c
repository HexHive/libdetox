#include <stdio.h>
#include "opcode_actions.h"

void default_action(int pos)
{
	printf("Default action called at address %d\n", pos);
}

void do_something_fancy(int pos)
{
	printf("Done something fancy at address %d\n", pos);
}

void do_something_cool(int pos)
{
	printf("Done something cool at address %d\n", pos);
}
