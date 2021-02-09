//-----------------------------------------------------------------------+
// File         :  set_pstat_stale.c                                     |
//                                                                       |
// Description  :  Takes care of setting the stale bits in CSRBPM PSTAT  |
//                 after data has been used.                             |
//                                                                       |
// Prototype    :  int cbpm_request()                                    |
//                                                                       |
// Arguments    :  None                                                  |
//                                                                       |
// Return Value :  F_SUCCESS (=0) or F_FAILURE (=1)                      |
//                                                                       |
// Author       :  M. Palmer                                             |
//                                                                       |
// Modifications:                                                        |
//                                                                       |
//                                                                       |
//-----------------------------------------------------------------------+

#include "standard_headers.h"


#define PSTAT_MAX_ELE 120


int set_pstat_stale_(void) {

  //   char *func_name = "set_pstat_stale";
  int ele;
  char *node = "CSRBPM PSTAT";
  int node_ele[2];
  int data[1];
  int istat = F_SUCCESS;
  
  //-----------------------------------------------------------------------+
  //                     EXECUTABLE CODE STARTS HERE                       |
  //-----------------------------------------------------------------------+
  
  for (ele = 1; ele <= PSTAT_MAX_ELE; ele++) {
    node_ele[0] = ele;
    node_ele[1] = ele;
    
    istat = vxgetn_c(READ, node, node_ele, data);
    
    if (istat == F_SUCCESS) {
      if (data[0] == CBPM_GOOD_DATA) {
	data[0] = CBPM_OLD_DATA;
	istat = vxputn_c(WRITE, node, node_ele, data);
	if (istat != F_SUCCESS) break;
      }
    } else {
      break;
    }
  }
  
  // Return status
  return istat;
}
