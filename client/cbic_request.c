/*!
*-------------------------------------------------------------------------+
* File         :  cbic_request.c                                          |
*                                                                         |
* Description  :  Populates the central MPM CBPM COMMAND distribution     |
*                 node structure with an action request and the           |
*                 associated parameters necessary for execution of an     |
*                 action.                                                 |
*                                                                         |
* Arguments    :  int  action      - Control action to execute               |
*                 int  mode        -                                          |
*                 int  num_bunches -                                         |
*                 int* species_list   -                                         |
*                 
*                                                                         |
* Author       :  M. Palmer / M. Rendina                                  |
*                                                                         |
*-------------------------------------------------------------------------+*/

#include "standard_headers.h"


//int cbpm_request(int action,        int mode,        int num_bunches, 
//                 int *species_list, int *train_list, int *bunch_list,
//                 int num_samples,   int num_turns,   int num_shots, 
//                 int turn_select) {

int cbic_request(int action,    int waitmode,     int num_samples, 
		 int num_turns, int *p_bunch_pat, int num_inj_shots,
		 int species                                         ) {

  char *func_name = __FUNCTION__;
  
  int bun, proc;          // loop variables
  int lock_status;        // hardware/software lock status
  int refresh_lock;       // hardware lock refresh flag
  int done  = DSP_FALSE;  // while loop variable
  int retry = 0;          // number of times to check command status
  // before timeout 
  int istat;
  int cbpm_species;
  int done_value;
  int error_value;
  
  int i, j, server;
  
  int num_proc = 0;
  int bad_server_table = DSP_FALSE;
  
  int  num_servers;
  int *p_num_servers = &num_servers;
 
  // Create a local copy of the interface data structure
  CBI_MPM_INTERFACE interface = {0};
  
  // pointers to data locations
  int *p_lock           = &(interface.lock);
  int *p_command        = &(interface.command);
  int *p_num_bunches    = &(interface.num_bunches);
  int *p_bunch_select   = &(interface.bunch_select[0]);
  int *p_num_samples    = &(interface.num_samples);
  int *p_num_cbpm_proc  = &(interface.num_cbpm_proc);
  int *p_server_ID_list = &(interface.cbpm_proc_id[0]);
  int *p_command_done   = &(interface.command_done[0]);
  int *p_heartbeat      = &(interface.heartbeat[0]);
  int *p_file_type      = &(interface.file_type[0]);
  int *p_file_id        = &(interface.file_id[0]);
  int *p_num_turns      = &(interface.num_turns);
  int *p_num_shots      = &(interface.num_shots);
  int *p_turn_select    = &(interface.turn_select);

  // Initialize variables specifying the "CBPM CONTROL" node
  char *node = CBPM_CONTROL_NODE;
  int   ele[2];
  int   hdw_lock_ele[2] = {ELE_LOCK, ELE_BID + MX_BUNCH_LIST - 1};
  
  // Status checking counts
  int ndone;
  int nworking;
  int nerror;
  
  // Timing information
  int to_sleep;
  int total_time;
  int last_refresh;
  
  // Return value variable
  int request_status = CBPM_REQUEST_SUCCESS;


  //-----------------------------------------------------------------------+
  // Set up hardware and software locks in order to take over control of   |
  // any SERVERS that are presently active and under control system control|
  //-----------------------------------------------------------------------+

  // Check for a hardware lock on the first control system entries of the 
  // CBPM CONTROL node
  
  lock_status = cbpm_lock(SET_LOCK, hdw_lock_ele);
  if (lock_status != CBPM_REQUEST_SUCCESS) {
    
    //    Log error message
    sprintf(message, "Unable to obtain control of existing SERVER\n");
    log_message(S_ERROR, func_name, message);
    
    //    Return error
    return CBPM_LOCK_ERROR;
  }
  


  //-----------------------------------------------------------------
  // Get the number of active CBPM processes and the process_id list
  //-----------------------------------------------------------------
  ele[0] = ELE_NUM_CBPM_PROC;
  ele[1] = ELE_NUM_CBPM_PROC;
  vxgetn_c(CBPM_CONTROL_NODE, ele[0], ele[1], p_num_cbpm_proc);

  num_proc = *p_num_cbpm_proc;
  printf("Number of servers found running = %d\n", num_proc);

  if ((num_proc < 1) || (num_proc > MAX_SERVERS)) {
    
    sprintf(message, 
	    "%d CBIC servers currently available (should have 1 - %1d)",
	    num_proc, MAX_SERVERS);
    log_message(S_ERROR, func_name, message);
    
    // Release all locks
    lock_status = cbpm_lock(RELEASE_LOCK, hdw_lock_ele);
    
    // Return error
    return CBPM_BAD_PROC_ERROR;
  }


  //-----------------------------------------------------------------
  // Check that the server identification table in the MPM node is
  // correctly constructed before proceeding.
  //-----------------------------------------------------------------
  ele[0] = ELE_CBPM_PROC_ID;
  ele[1] = ELE_CBPM_PROC_ID + MAX_SERVERS - 1;
  vxgetn_c(CBPM_CONTROL_NODE, ele[0], ele[1], p_num_servers);
  
  bad_server_table = DSP_FALSE;
  for (i = 0; i < num_servers; i++) {
    for (j = i+1; j < num_servers; j++) {
      if (*(p_server_ID_list + i) == *(p_server_ID_list + j)) {
	bad_server_table = DSP_TRUE;
      }
    }
  }


  if (bad_server_table == DSP_TRUE) {

    sprintf(message, "Corrupt server list!");
    log_message(S_ERROR, func_name, message);
    for (server = 0; server < num_servers; server++) {
      sprintf(message, "Server ID %2d = %15d", proc, 
	      *(p_server_ID_list + server)); 
      log_message(S_BLANK, func_name, message);
    }
    
    //    Release all locks
    lock_status = cbpm_lock(RELEASE_LOCK, hdw_lock_ele);

    return CBPM_BAD_PROC_ERROR;

  }
  
  
  

  

  //-----------------------------------------------------------------------+
  // Now have hardware and software locks.  First clear any stale replies  |
  // from the CBPM process command_done array.  Then set up the new        |
  // command inputs.                                                       |
  //-----------------------------------------------------------------------+
  for (proc = 0; proc < MAX_SERVERS; proc++) {
    *(p_command_done + proc) = CBPM_CMD_CLEAR;
  }     
 
  ele[0] = ELE_COMMAND_DONE;
  ele[1] = ele[0] + MAX_SERVERS - 1;
  vxputn_c(CBPM_CONTROL_NODE, ele[0], ele[1], p_command_done);
  
  *p_command     = action;
  
  //*p_num_bunches = num_bunches;
  //*p_num_samples = num_samples;
  //*p_num_turns   = num_turns;
  //*p_num_shots   = num_shots;
  //*p_turn_select = turn_select;
  
  //--------------------------
  // Send the bunch pattern
  //--------------------------
  ele[0] = ELE_BID;
  ele[1] = ELE_BID + 40;
  int fives[40] = {5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5};
  for (i = 0; i < 40; i++) {
    printf("bunpat[%i] = %d\n", i, *(p_bunch_pat+i) );
  }
  vxputn_c(CBPM_CONTROL_NODE, ele[0], ele[1], p_bunch_pat);
  //vxputn_c(CBPM_CONTROL_NODE, ele[0], ele[1], &fives);


  ele[0] = ELE_COMMAND;
  ele[1] = ELE_COMMAND;
  //-----------------------------------------------------
  // SEND THE COMMAND, THUS TRIGGERING THE SERVER TO ACT
  //-----------------------------------------------------
  vxputn_c(CBPM_CONTROL_NODE, ele[0], ele[1], p_command);

  /*
  for (bun = 0; bun < num_bunches; bun++) {  // necessary?  Break out into checker functions.
    
    //    Convert species value and check validity of inputs
    if (species_list[bun] == CESR_ELECTRON) {
      cbpm_species = ELECTRONS;
    } else if (species_list[bun] == CESR_POSITRON) {
      cbpm_species = POSITRONS;
    } else {
      sprintf(message, "Bad species value (e- = -1, e+ = +1): %d", 
	      species_list[bun]);
      log_message(S_ERROR, func_name, message);
      
      //       Release all locks
      lock_status = cbpm_lock(RELEASE_LOCK, hdw_lock_ele);
      
      //       Return error
      return CBPM_INPUT_ERROR;
    }
    
    //    Now check train value - necessary?
    if ((train_list[bun] < 1) || (train_list[bun] > NUM_TRAINS)) {
      sprintf(message, "Bad CESR train value (1 - %1d): %d", 
	      NUM_TRAINS, train_list[bun]);
      log_message(S_ERROR, func_name, message);
      
      //       Release all locks
      lock_status = cbpm_lock(RELEASE_LOCK, hdw_lock_ele);
      
      //       Return error
      return CBPM_INPUT_ERROR;   
    }
    
    //    Now check bunch value - necessary?
    if ((bunch_list[bun] < 1) || (bunch_list[bun] > BUNCHES_PER_TRAIN)) {
      sprintf(message, "Bad CESR bunch value (1 - %1d): %d", 
	      BUNCHES_PER_TRAIN, train_list[bun]);
      log_message(S_ERROR, func_name, message);
      
      //       Release all locks
      lock_status = cbpm_lock(RELEASE_LOCK, hdw_lock_ele);
      
      //       Return error
      return CBPM_INPUT_ERROR;   
    }
    
    //    Convert inputs to bunch IDs                        FIXME
    //*(p_bunch_id + bun) = cbpm_get_bid(cbpm_species, 
    //			       train_list[bun], bunch_list[bun]);
  }
  
  */
  
  return F_SUCCESS;

}
