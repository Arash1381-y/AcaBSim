#include <math.h>
#include <stdlib.h>
#include <utils.h>

#define RATE 0.25

int
exponential_sample ()
{
  double lambda = RATE; // rate parameter
  return (int)(-log (1.0 - (double)rand () / RAND_MAX) / lambda) + 1;
}

int
unifrom_sample (int a, int b)
{
  return a + (int)(((double)rand () / RAND_MAX) * (b - a + 1));
}