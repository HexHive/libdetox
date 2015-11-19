#include <stdio.h>
#include <libfastbt.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	volatile int i = 3;
	int res = 0;
	int j;

	fbt_init(NULL);
	fbt_start_transaction(&fbt_commit_transaction);

	for(j=0;j<10000;j++)
	{
		i = rand() % 7;
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
			res = 3;
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
	}




	for(j=0;j<10000;j++)
	{
		i = rand() % 7;
		printf("switch start\n");

		switch(i)
		{
			case 0:
			printf("1\n");
			break;
			case 1:
			printf("2\n");
			break;
			case 2:
			printf("3\n");
			break;
			case 3:
			printf("4\n");
			res = 3;
			break;
			case 4:
			printf("5\n");
			break;
			case 5:
			printf("6\n");
			break;
			case 6:
			printf("7\n");
			break;
			case 7:
			printf("8\n");
			break;
			default:
			printf("default case\n");
		}

		printf("switch end\n");
	}





	fbt_commit_transaction();
	printf("value of i: %d\n", i);
	return 0;
}


