/*!
*-------------------------------------------------------------------------+
* File         :  take_cbpm_orbit.c                                       |
*                                                                         |
* Description  :  Instructs an instance of a CBIC server to obtain orbit  |
*                 data with the parameters specified.                     |
*                 Must be connected to a CBIC server process to use.      |
*                                                                         |
* Arguments    :  
*                 
*                                                                         |
* Author       :  M. Palmer / M. Rendina                                  |
*                                                                         |
*-------------------------------------------------------------------------+*/

#include "standard_headers.h"


int take_cbpm_orbit( int species, int train, int *p_bunch, int *p_waitmode) {

  //char *func_name = __FUNCTION__;

  int istat;
  int i;
  int action, waitmode, num_bunches, num_samples, num_turns;
  int num_shots, turn_select;
  
  //----------------------------------------------
  action = 25;  // FIXME: change to take orbit action named constant
  //----------------------------------------------

  num_bunches = 1;
  
  // User arguments
  // Note: (species,train,bunch) pointers simply get passed through
  waitmode = *p_waitmode;
  
  num_samples = 1;
  num_turns   = 1024;
  num_shots   = 0;
  turn_select = 0;
  
  //-----------------------------------------------------------
  // Convert the Bunch ID value here into a full bunch pattern
  // appropriate to the bunch spacing mode that is active
  // 183 or 640 bunches total.
  //-----------------------------------------------------------
  int spacing_mode = 14;
  int bbucket = 0;
  bbucket = bunch_number( spacing_mode, train, *p_bunch );
  printf("\nTRAIN-%d, BUNCH-%d for this species becomes BUCKET-%d\n\n", \
	 train, *p_bunch, bbucket);
  
  //------------------------------------------------------------
  // Create a suitable bunch pattern using this information (!)
  //------------------------------------------------------------
  // FIXME: HARDCODED TO FIRST BUNCH BUCKET!  (0) (T1,B1)
  int bunch_pattern[40];
  // Initialize bunch pattern to avoid garbage
  for (i = 0; i < 40; i++) {
    bunch_pattern[i] = 0;
  }
  int *p_bunch_pat;
  p_bunch_pat = &(bunch_pattern);
  
  bunch_pattern[0] = 1;


  int num_inj_shots = 0; // To be used later?

  
  sprintf(message, "Sending CBIC ACTION request");
  log_message(S_INFO, __FUNCTION__, message);
  
  istat=cbic_request(action,     waitmode,     num_samples, 
		     num_turns,  p_bunch_pat,  num_inj_shots,
		     species );
  
  return istat;
  
}
