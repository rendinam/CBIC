/*!
*-------------------------------------------------------------------------+
* File         :  bunch_number.c                                          |
*                                                                         |
* Description  :  Returns the bunch bucket number (indexed from 0)for the |
*                 given train and bunch identifier in the specified bunch |
*                 spacing mode.                                           |
*                                                                         |
* Arguments    :  int  spacing_mode - 4ns or 14ns                         |
*                 int  train        - train number                        |
*                 int  bunch        - bunch number                        |
*                                                                         |
* Author       :  M. Rendina                                              |
*-------------------------------------------------------------------------+*/

#include "standard_headers.h"


int bunch_number( int spacing_mode, int train, int bunch ) {
  
  int bunch_number;

  if (spacing_mode == 14) {
    bunch_number = (train - 1) * 20  +  (int)((train-1)/3)  +  bunch_number - 1;
  }

  return bunch_number;

}
