#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "libfastbt.h"

#define NUM_ITER 1000000

void filetest() {
  int i;
  for (i = 0; i < NUM_ITER; ++i) {
    if (access(".", R_OK|W_OK) == 0) {
      int fw = creat("./test.dat", 660);
      if (fw == -1) perror("Creat error");
      int fr = open("./input.dat", 0);
      if (fr == -1) perror("Open error");
      close(fw);
      close(fr);
    } else {
      printf("Unable to open dir\n");
    }
  }
}

void filetest2() {
  int i, j;
  for (i = 0; i < NUM_ITER; ++i) {
    j += access(".", R_OK|W_OK);
  }
}

void filetest3() {
  int i, j;
  for (i = 0; i < NUM_ITER; ++i) {
    j = open("./input.dat", 0);
    close(j);
  }
}

int main() {
  struct timeval start, stop;
  double total;

  struct thread_local_data *tld = fbt_init(NULL);
  fbt_start_transaction(tld, fbt_commit_transaction);
  gettimeofday(&start, 0);
  filetest();
  gettimeofday(&stop, 0);
  fbt_commit_transaction();
  fbt_exit(tld);

  total = stop.tv_sec - start.tv_sec;
  if (stop.tv_usec - start.tv_usec < 0) total++;
  total *= 1000*1000;
  total += stop.tv_usec - start.tv_usec;
  total /= (NUM_ITER);
  printf("access, open, creat, close, close %10f ms\n", total);

  tld = fbt_init(NULL);
  fbt_start_transaction(tld, fbt_commit_transaction);
  gettimeofday(&start, 0);
  filetest2();
  gettimeofday(&stop, 0);
  fbt_commit_transaction();
  fbt_exit(tld);

  total = stop.tv_sec - start.tv_sec;
  if (stop.tv_usec - start.tv_usec < 0) total++;
  total *= 1000*1000;
  total += stop.tv_usec - start.tv_usec;
  total /= (NUM_ITER);
  printf("access %10f ms\n", total);

  tld = fbt_init(NULL);
  fbt_start_transaction(tld, fbt_commit_transaction);
  gettimeofday(&start, 0);
  filetest3();
  gettimeofday(&stop, 0);
  fbt_commit_transaction();
  fbt_exit(tld);

  total = stop.tv_sec - start.tv_sec;
  if (stop.tv_usec - start.tv_usec < 0) total++;
  total *= 1000*1000;
  total += stop.tv_usec - start.tv_usec;
  total /= (NUM_ITER);
  printf("open, close %10f ms\n", total);

  gettimeofday(&start, 0);
  filetest();
  gettimeofday(&stop, 0);
  total = stop.tv_sec - start.tv_sec;
  if (stop.tv_usec - start.tv_usec < 0) total++;
  total *= 1000*1000;
  total += stop.tv_usec - start.tv_usec;
  total /= (NUM_ITER);
  printf("access, open, creat, close, close %10f ms\n", total);

  gettimeofday(&start, 0);
  filetest2();
  gettimeofday(&stop, 0);
  total = stop.tv_sec - start.tv_sec;
  if (stop.tv_usec - start.tv_usec < 0) total++;
  total *= 1000*1000;
  total += stop.tv_usec - start.tv_usec;
  total /= (NUM_ITER);
  printf("access %10f ms\n", total);

  gettimeofday(&start, 0);
  filetest3();
  gettimeofday(&stop, 0);
  total = stop.tv_sec - start.tv_sec;
  if (stop.tv_usec - start.tv_usec < 0) total++;
  total *= 1000*1000;
  total += stop.tv_usec - start.tv_usec;
  total /= (NUM_ITER);
  printf("open, close %10f ms\n", total);
return 0;
}
