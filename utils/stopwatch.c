//-----------------------------------------------------------------------+
// File         :  stopwatch.c                                           |
//                                                                       |
// Description  :  Functions for timing code execution (in wall clock    |
//                 time with sub-second resolution.                      |
//                                                                       |
// Arguments    :  None                                                  |
//                                                                       |
// Author       :  M. Rendina                                            |
//                                                                       |
//-----------------------------------------------------------------------+

#include "standard_headers.h"

double t_start() {
  
  int status;
  struct timeval time;
  status = gettimeofday( &time, NULL );
  printf("           -=-=-=-=-  TIMER  START -=-=-=-=- %f \n", time.tv_usec );
  
  return time.tv_usec;
}


double t_stop(double start_usecs) {
  
  int status;
  double diff;
  struct timeval time;
  
  status = gettimeofday( &time, NULL );
  diff = time.tv_usec - start_usecs;
  printf("           -=-=-=-=-  TIMER  STOP  -=-=-=-=-  [ %fs ]\n", diff/1000000 );
  
  return diff;
}
