/*!
*-----------------------------------------------------------------------+
* File         :  unpack.c                                              |
*                                                                       |
* Description  :  Returns the requested byte of the given word.         |
*                                                                       |
*                                                                       |
* Arguments    :  The word to unpack                                    |
*                 Which byte to extract                                 |
*                                                                       |
*                                                                       |
* Author       :  M.Rendina                                             |
*                                                                       |
*-----------------------------------------------------------------------+*/

#include "standard_headers.h"


int unpack( int word, int byte ) {

  int mask = 0x000000FF; // lowest 8 bits high
  mask = mask << (byte*8);

  return ( (word & mask) >> (byte*8) );

}
