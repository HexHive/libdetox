#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>

void childDieHandler(int sig, siginfo_t *child_info, void *context);

int
main()
{
    sigset_t blockMask, emptyMask;
    struct sigaction sa;

    bzero(&sa, sizeof(sa));
    sa.sa_sigaction = childDieHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGCHLD, &sa, NULL);

    setbuf(stdout, NULL);       /* Disable buffering of stdout */

    sigemptyset(&blockMask);
    sigaddset(&blockMask, SIGCHLD);
    if (sigprocmask(SIG_SETMASK, &blockMask, NULL) == -1) return 0;

    if(fork() == 0) {

      printf("child: forked.\n");
      sleep(5);
      exit(0);

    } else {

     printf("parent waiting...\n");
     sigemptyset(&emptyMask);
     sigsuspend(&emptyMask);
     printf("parent suspend over.");

    }

    printf("parent done\n");
    return 0;
}

void childDieHandler(int sig, siginfo_t *child_info, void *context){
    int status;
    pid_t childPid;
/*
    while((childPid = waitpid(-1,&status, WNOHANG)) > 0) {
        int pid = (int) childPid;
        fprintf(logFile,"GP:*** PROCESS KILLED [pid %d]\n",pid);

    sigset_t set;
    sigpending(&set);
    if(sigismember(&set, SIGCHLD)){
        fprintf(logFile, "GP: SIGCHLD is pending or blocked!!!!\n");
        fflush(logFile);
    }

        fflush(logFile);

        // identify exited process and then restart it
        if(currentChildPid == childPid){
        // kill any child 
        system("killall child");
        fprintf(logFile,"GP: Restarting parent process...\n");
        fflush(logFile);
        startProcess("parent");
    }

    }

    fprintf(logFile,"GP:End of childDieHandler()... [%d]\n\n",(int)childPid);
    fflush(logFile);
*/

   printf("childDieHandler: SIGCHLD caught!\n");

}

