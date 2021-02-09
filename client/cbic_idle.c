/*!
*-------------------------------------------------------------------------+
* File         :  cbic_idle.c                                             |
*                                                                         |
* Description  :  Instructs an instance of a CBIC server to run the idle  |
*                 action with output local to the server terminal.        |
*                                                                         |
* Arguments    :  
*                 
* Author       :  M. Rendina                                              |
*                                                                         |
*-------------------------------------------------------------------------+*/

#include "standard_headers.h"


int cbic_idle( ) {

  int fstat;

  int action = 1;

  int waitmode      = 0;
  int num_samples   = 0;
  int num_turns     = 0;
  int bun_pat[40];
  int num_inj_shots = 0;
  int species       = 0;

  fstat = cbic_request( 1, waitmode, num_samples, num_turns, bun_pat, num_inj_shots, species );

  return F_SUCCESS;

}
