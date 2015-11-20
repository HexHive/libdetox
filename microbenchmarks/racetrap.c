#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "libfastbt.h"

int main() {
  struct thread_local_data *tld = fbt_init(NULL);
  fbt_start_transaction(tld, fbt_commit_transaction);
  char buf[16];
  int fd;
  if (access("file", R_OK) != 0) {
    return 1;
  }
  /* race opportunity */
  sleep(1);
  
  fd = open("file", O_RDONLY);
  int len = read(fd, buf, 16);
  buf[len] = 0x0;
  printf("read: '%s' (%d), expected: 'foo'\n", buf, len);
  fflush(0);
  fbt_commit_transaction();
  fbt_exit(tld);
  return 0;
}
