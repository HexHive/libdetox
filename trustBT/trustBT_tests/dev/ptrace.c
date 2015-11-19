#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <sys/syscall.h>

int main()
{   pid_t child;
    const int long_size = sizeof(long);
    child = fork();
    if(child == 0) {
        //ptrace(PTRACE_TRACEME, 0, NULL, NULL);
/*
    int i;
    for(i = 0;i < 10; ++i) {
        printf("My counter: %d\n", i);
        sleep(2);
    }
    return 0;
*/
	//execl("/bin/ls", "ls", NULL);
	sleep(5);
	getpid();
        //sleep(1);
	return 0;
    }
    else {
        int status;
        union u {
            long val;
            char chars[long_size];
        }data;
        struct user_regs_struct regs;
        int start = 0;
        long ins;
	//wait(NULL);
    	ptrace(PTRACE_ATTACH, child, NULL, NULL);
	sleep(2);
            ptrace(PTRACE_GETREGS,
                   child, NULL, &regs);

                ins = ptrace(PTRACE_PEEKTEXT,
                             child, regs.eip,
                             NULL);
                printf("EIP: %lx Instruction "
                       "executed: %lx\n",
                       regs.eip, ins);
/*
        while(1) {
            wait(&status);
            if(WIFEXITED(status))
                break;
            ptrace(PTRACE_GETREGS,
                   child, NULL, &regs);

                ins = ptrace(PTRACE_PEEKTEXT,
                             child, regs.eip,
                             NULL);
                printf("EIP: %lx Instruction "
                       "executed: %lx\n",
                       regs.eip, ins);


        }
*/
    }
    return 0;
}
