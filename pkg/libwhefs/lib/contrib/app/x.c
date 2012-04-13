#include <stdio.h>

int main()
{
  FILE * f = fopen("/dev/stdout", "w+");
  fclose(f);
  return 0;
}
