/*!
*-----------------------------------------------------------------------+
* File         :  get_hexint.c                                          |
*                                                                       |
* Description  :  Waits until user enters a keystroke and returns it    |
*                 as an integer.                                        |
*                                                                       |
*                                                                       |
* Arguments    :  None                                                  |
*                                                                       |
* Author       :  M.Rendina                                             |
*                                                                       |
*-----------------------------------------------------------------------+*/

#include "standard_headers.h"


int get_hexint() {
  char text[20];
  fflush(stdout);
  if ( fgets(text, sizeof text, stdin) ) {
    int number;
    if ( sscanf(text, "%x", &number) == 1 ) {
      return number;
    }
  }
  return -999999;
}
