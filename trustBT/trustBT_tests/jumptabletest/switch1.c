#include <stdio.h>

int main()
{
	volatile int i = 1;
	printf("switch start\n");
	switch(i)
	{
		case 0:
		printf("case 0\n");
		break;
		case 1:
		printf("case 1\n");
		break;
		case 2:
		printf("case 2\n");
		break;
		case 3:
		printf("case 3\n");
		break;
		case 4:
		printf("case 4\n");
		break;
		case 5:
		printf("case 5\n");
		break;
		case 6:
		printf("case 6\n");
		break;
		case 7:
		printf("case 7\n");
		break;
		default:
		printf("default case\n");
	}
	printf("switch end\n");
	return 0;
}
