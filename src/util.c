#include "util.h"

#include <stddef.h>
#include <sys/time.h>
    
void get_walltime_(double* wcTime) {
  struct timeval tp;
  gettimeofday(&tp, NULL);
  *wcTime = (double)(tp.tv_sec + tp.tv_usec/1000000.0);
}

double get_wtime(void) {
  double wcTime;
  get_walltime_(&wcTime);
  return wcTime;
}

int get_num_omp_threads(void) {
  int num_threads = 0;
  #pragma omp parallel reduction(+:num_threads)
  num_threads += 1;
  return num_threads;
}
