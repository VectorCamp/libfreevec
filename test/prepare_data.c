#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "config.h"

int main() {
  int s;
  char r[RANDOMSIZE];

  for (s=0; s < RANDOMSIZE; s++) {
    r[s] = 1+ (char) (254.0*(float)rand()/(float)RAND_MAX);
  }

  int randomdata = creat("random.dat", S_IRWXU);
  if (randomdata != -1) {
    write(randomdata, &r, RANDOMSIZE);
    close(randomdata);
  } else
    printf("error creating file\n");
}
