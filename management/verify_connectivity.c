/*!
*-----------------------------------------------------------------------+
* File         :  verify_connectivity.c                                 |
*                                                                       |
* Description  :  Compares the instrument hostname obtained from the    |
*                 instrument-specific parameter file with the values    |
*                 read back via XBUS and ETHERNET methods.  Returns     |
*                 TRUE if all values match, FALSE otherwise.            |
*                                                                       |
* Arguments    :  Pointer to holding array of master control structs to |
*                                                                       |
* Author       :  M.Rendina                                             |
*-----------------------------------------------------------------------+*/

#include "standard_headers.h"

#define    COM_TRIES                2
#define    BOOTUP_WAIT_SECS         20
#define    ACTIVITY_RETRY_FLAG      999
#define    POWER_SUPPLY_WAIT_SECS   4

int verify_connectivity( void ) {
  
  char *func_name = (char*)__FUNCTION__;

  int debug_level = CTL_System.debug_verbosity;

  CBPM_DATA          *dp;

  int  inst, inst2, elem, elem2, power_cyc_elem, value, numINSTs = CTL_System.n_Modules;
  int  PowerRelayEnabled;
  int  looped_for_power = CBI_FALSE;
  int  reset_requested = CBI_FALSE;
  int  status, xb_status, pingstatus, try, bootup_tally = 0;
  char command[50];

  char *error_buf;
  int socketfd = 0;
  int commstatus;

  // Skip all communications steps if in highest maintenance mode level.
  if (maint_mode < 3) {


    for (try = 0; try < COM_TRIES; try++) {
      
      if (try != 0 && bootup_tally > 0) {
	printf("\n\n**************************************************************\n");
	printf(" Some instruments were soft-reset in the first pass.  Now\n");
	printf(" waiting %d seconds for these to complete their boot process.\n", BOOTUP_WAIT_SECS);
	printf("**************************************************************\n\n\n");
	sleep(BOOTUP_WAIT_SECS);
      }
      
      bootup_tally = 0;
      
      for (inst = 0; inst < CTL_System.n_Modules; inst++) {
	
	if (try == 0) {  // First time through, initialize each socket ID value.
	  CTL_System.p_Module[inst]->comm.ethernet.socket_ID = CBI_SOCKET_CLOSED;
	}

	if (CTL_System.p_Module[inst]->active == CBI_EMPTY_SLOT ||
	    CTL_System.p_Module[inst]->active == ACTIVITY_RETRY_FLAG)  {
	  
	  
	  dp   = CTL_System.p_Module[inst]->dsp_data;
	  elem = CTL_System.p_Module[inst]->comm.xbus.element;
	  
	  
	  //-------------------------------------------------------------------
	  // Attemp to communicate with instrument.
	  // Make COM_TRIES number of tries with low-level reset attempts
	  // inbetween any failures encountered.  If this exceeds the number
	  // of allotted tries, place the instrument into 'maintenance' mode
	  // to provide for low level maintenance operations later.
	  //-------------------------------------------------------------------
	  printf("\n\n-------- Verifying connectivity for instrument %s --------\n", 
		 CTL_System.p_Module[inst]->comm.ethernet.hostname );
	  printf(" %s  %d, ",
		 CTL_System.p_Module[inst]->comm.xbus.pkt_node,
		 CTL_System.p_Module[inst]->comm.xbus.element );
	  
	  
	  
	  // Process/propagate XBUS addressing info.
	  // 1) Strip out underscores until faccess is intelligent enough to acquire quoted strings.
	  // 2) Replace substrings as necessary to turn PKT node name into ADR and DAT node names.
	  replace( CTL_System.p_Module[inst]->comm.xbus.pkt_node, '_', ' ' );
	  strcpy( CTL_System.p_Module[inst]->comm.xbus.adr_node, 
		  (char *)replace_str(CTL_System.p_Module[inst]->comm.xbus.pkt_node, "PKT", "ADR") );
	  strcpy( CTL_System.p_Module[inst]->comm.xbus.dat_node, 
		  (char *)replace_str(CTL_System.p_Module[inst]->comm.xbus.pkt_node, "PKT", "DAT") );
	  
	  
	  // Obtain hostname via XBUS packet operation and compare -   DON'T NEED THIS. WILL LATER DO THE SAME TEST WITH ETHERNET - more reliable
	  /*xb_status = cbi_gp_struct(READ, XBUS, CBI_IDENT_TAG, CTL_System.p_Module[inst], CBI_FULL_STRUCT);
	  if (xb_status == CBI_F_SUCCESS) {
	    printf("Read IDENT via XBUS\n");
	    if (strcmp(CTL_System.p_Module[inst]->comm.ethernet.hostname, (char *)CTL_System.p_Module[inst]->core->cbi_ident.hostname) != 0) {
	      xb_status = CBI_F_FAILURE;
	      printf("Hostname read back via XBUS does not match the assignment specified in parameter file!\n");
	    }
	  } else { 
	    printf("ERROR obtaining indentification info via XBUS!\n");
	  }*/

	  // Perform a simple PING test here.  If this fails, there is no need to attempt to get
	  // the hostname via ethernet.
	  // For ease of implementation, use a shell call to perform the ping.  
	  // If greater compatibility/portability is needed, convert this all to use raw 
	  // socket code.  This will be more complex as a result.
	  // 'ping' command arguments:
	  //                           -w 1    -- soft maximum timeout of 1s
	  //                           -c 1    -- packet count of 1
	  //                           -i 0.2  -- packet interval of 0.2s
	  // Throw away all output by redirecting to /dev/null to avoid cluttering display.
	  printf("Ping test to [%s]...", CTL_System.p_Module[inst]->comm.ethernet.hostname );
	  sprintf(command, "/bin/ping -w 1 -c 1 -i 0.2 %s > /dev/null", CTL_System.p_Module[inst]->comm.ethernet.hostname);
	  status = system(command);
	  if (status != 0) {
	    perror("  system(ping) ");
	  }


	  // If the ping succeeded, try to get the identification info from the instrument.
	  if (status == 0) {
	    printf("OK, ");


	    // Low-level communications setup (CBInet)
	    cbi_net_clr_error(); 
	    if (debug_level > CBI_DEBUG_LEVEL_3) {
	      printf("Opening socket to [%s]...\n", CTL_System.p_Module[inst]->comm.ethernet.hostname);
	    }
	    socketfd = cbi_net_fdopen( CTL_System.p_Module[inst]->comm.ethernet.hostname );
	    if (socketfd > 0) {
	      // Save the socket number in the central structure tree for state management.
	      CTL_System.p_Module[inst]->comm.ethernet.socket_ID = socketfd;
	      // Obtain hostname via ETHERNET
	      status = cbi_gp_struct(READ, ETHERNET, CBI_IDENT_TAG, CTL_System.p_Module[inst], CBI_FULL_STRUCT);
	      printf("Read IDENT via ETHERNET, ");
	      if (status == CBI_F_SUCCESS) {
		if (strcmp(CTL_System.p_Module[inst]->comm.ethernet.hostname, 
			   (char *)CTL_System.p_Module[inst]->core->cbi_ident.hostname) != 0) {
		  printf("Hostname read back via ETHERNET does not match the assignment specified in parameter file!\n");
		  status = CBI_F_FAILURE;
		}
	      }
	    } else {
	      error_buf = cbi_net_get_error();
	      sprintf(message, "Bad socket fd. Unable to connect to remote host - %s \n   cbi_net_error_buf: %s", 
		      CTL_System.p_Module[inst]->comm.ethernet.hostname,
		      error_buf );
	      cbi_net_clr_error();
	      log_message( S_ERROR, func_name, message );
	      status = CBI_F_FAILURE;
	    }



	  } else {
	    printf("FAILED! (system() call returned status %d)\n", status); // endIF (status == 0)
	    if (status == -1) {
	      printf("  Problem spawning shell for ping.  Check that there are no zombie\n");
	      printf("  instances of this program running under the same user name.\n");
	    }
	  }

	  
	  if ( status == F_SUCCESS) {
	    printf("   Identification Read \033[32;1mOK\033[0m \n");
	    fflush(stdout);
	    // Copy TCP/IP addressing info to core components of data struct tree
	    sprintf( (char *)CTL_System.p_Module[inst]->comm.ethernet.IP_address, 
		     (char *)CTL_System.p_Module[inst]->core->cbi_ident.ipaddr);
	    CTL_System.p_Module[inst]->active = CBI_ACTIVE;

	  }  else {  // If the ping or hostname retrieval failed...
	    
	    printf("Communication attempt to     %s (%s %d)  @  %s    \033[1;37;41mFAILED!\033[0m \n",
		   CTL_System.p_Module[inst]->comm.ethernet.hostname,
		   CTL_System.p_Module[inst]->comm.xbus.pkt_node,
		   CTL_System.p_Module[inst]->comm.xbus.element,
		   CTL_System.p_Module[inst]->det.location            );
	    // In the event of a failure, send the ON command to ALL instrument's 
	    // power supply relays.  Wait for some seconds for the power to stabilize, 
	    // and then continue with individual instruments.
	    // Add support for bulk power supply mapping here?
	    if (looped_for_power == CBI_FALSE) {
	      printf("\nEnsuring all power supply relays are in the \'ON\' state.\n");
	      looped_for_power = CBI_TRUE;
	      for (inst2 = 0; inst2 < CTL_System.n_Modules; inst2++) {
		elem2          = CTL_System.p_Module[inst2]->comm.xbus.element;
		power_cyc_elem = elem2;
		PowerRelayEnabled = 1;
		if (CTL_System.p_Module[inst2]->comm.xbus.adr_node[9] == 'T') {
		  //printf("\'TST\' instrument.  Power cycle element adjusted accordingly.\n");
		  PowerRelayEnabled = 0;
		}
		
		if (CTL_System.p_Module[inst2]->active != ACTIVITY_RETRY_FLAG) {
		  value = 1;
		  if (PowerRelayEnabled) {
		  	status = vxputn_c(CBI_POWER_CYCLE_NODE, power_cyc_elem, power_cyc_elem, &value);
		    if (status != MPM_SUCCESS) {
		      printf("vxputn to make sure power supply is turned on for node %s ele %d FAILED!\n", 
			     CBI_POWER_CYCLE_NODE, 
			     power_cyc_elem);
		    }		  	
		  }
		}
	      }
	      // One-time wait to allow power supplies to stabilize.
	      printf("Waiting %d seconds for power to stabilize.\n", POWER_SUPPLY_WAIT_SECS);
	      sleep(POWER_SUPPLY_WAIT_SECS);
	    }

	    printf("Attempting XBUS reboot of instrument %s (%s) ele %d...\n", 
		   CTL_System.p_Module[inst]->comm.ethernet.hostname,
		   CTL_System.p_Module[inst]->comm.xbus.dat_node,
		   elem);
	    reset_requested = CBI_TRUE;
	    
	    
		  int status = cbi_xbus_poke( inst, DSP_RESET, 0);
		  if (status != CBI_F_SUCCESS) {

		    sprintf(message, "MPMnet xbus reset write of 0 to address 0x%x for %s FAILED!",
			    DSP_RESET,
			    CTL_System.p_Module[inst]->comm.ethernet.hostname );
		    log_message(S_ERROR, func_name, message);
		    return CBI_F_FAILURE;
		  }
  
		  status = cbi_xbus_poke( inst, DSP_RESET, 1);
		  if (status != CBI_F_SUCCESS) {
    
		    sprintf(message, "MPMnet xbus reset write of 1 to address 0x%x for %s FAILED!",
			    DSP_RESET,
			    CTL_System.p_Module[inst]->comm.ethernet.hostname );
		    log_message(S_ERROR, func_name, message);
		    return CBI_F_FAILURE;
		  }
  

	    if (try == COM_TRIES-1) { // Final attempt failed
	      printf("    Make sure the instrument has been flashed with a functional software image\n");
	      printf("    and that the XBUS node name matches the IP address for each line\n");
	      printf("    in the instrument list.\n\n\n");
	      CTL_System.p_Module[inst]->active =  CBI_INACTIVE;
	      //  break;
	    } else {
	      printf("Flagging instrument for retry...\n");
	      bootup_tally++;
	      CTL_System.p_Module[inst]->active  = ACTIVITY_RETRY_FLAG;
	    }
	    fflush(stdout);	    

	  } // endIF  status


	  if (CTL_System.p_Module[inst]->comm.ethernet.socket_ID != CBI_SOCKET_CLOSED) {
	    commstatus = cbi_net_close_socket(CTL_System.p_Module[inst]->comm.ethernet.socket_ID);
	  
	    // Close local end of socket
	    cbi_net_net_close(CTL_System.p_Module[inst]->comm.ethernet.socket_ID);
	    if (debug_level > CBI_DEBUG_LEVEL_3) {
	      sprintf(message, "Closing socket %d to %s", 
		      CTL_System.p_Module[inst]->comm.ethernet.socket_ID, 
		      CTL_System.p_Module[inst]->comm.ethernet.hostname   );
	      log_message( S_INFO, message, func_name );
	    }
	    CTL_System.p_Module[inst]->comm.ethernet.socket_ID = CBI_SOCKET_CLOSED;
	  }


	} //endIF active
	

       
      } //endFOR inst
      
  
    } //endFOR try
  
  
  } // endIF maint_mode


  return CBI_F_SUCCESS;
  
}
