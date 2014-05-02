#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

double getSeconds() {
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return (double) (tp.tv_sec + ((1e-6)*tp.tv_usec));
}

int main(int argc, char *argv[])
{
  int count = 0;
  double elapsedTime = getSeconds();
  // Do something
  int max = 1000000000;
  if (argc > 1)
    max = atoi(argv[1]);
  int i;
  for (i = 0; i < max; i++)
    count++;
  elapsedTime = getSeconds() - elapsedTime;
  printf("Count: %d\nElapsed time: %f\n", count, elapsedTime);
  return 0;
}
