/*!
*-------------------------------------------------------------------------+
* File         :  trig_bpms.c                                             |
*                                                                         |
* Description  :  Program to change the stat of the "A" or "B" triggering |
*                 bits in the phase word that gets encoded on the timing  |
*                 signal.                                                 |
*                                                                         |
* Author       :  M. Rendina                                              |
*                                                                         |
*-------------------------------------------------------------------------+*/

#include "standard_headers.h"


#define USE_MNET_MANAGER

int main (void) {
  
  int istat;

  char *node = "CBPM COMMAND";
  int option = -1;
  int ele[2];

  int trig_enable_value = 2;
  int trig_reset = 0;

  istat = init_comms();
  if (istat == F_FAILURE) {
    printf("Communications init failure!\n");
    exit(1);
  }


  do { 
    printf("  -- Digital BPM System Triggering Utility --  \n\n");
    printf("1) Trigger (with reset)\n");
    printf("2) Trigger  (no reset)\n");
    printf("\n0) EXIT\n");
    printf("--------------------------------\n");
    printf("Enter command: ");
    option = get_int();
    
    if (option == 1) {
      printf("Triggering...\n");
      ele[0] = 1;
      ele[1] = 1;
      vxputn_c(node, ele[0], ele[1], &trig_enable_value );
      sleep(1.5);
      vxputn_c(node, ele[0], ele[1], &trig_reset );
      printf("Reset.\n");
    }

    if (option == 2) {
      printf("Triggering...\n");
      ele[0] = 1;
      ele[1] = 1;
      vxputn_c(node, ele[0], ele[1], &trig_enable_value );
      sleep(1.5);
    }

    if (option == 0) {
        close_MPMnet_connection();
    }

  } while ( option != 0 );


  return F_SUCCESS;

}
