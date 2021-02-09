#ifndef CBIC_TYPES_H
#define CBIC_TYPES_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "utilities.h"
#include "cbi_core_includes.h"
#include "cbic_params.h"

//-----------------------------------------------------------------------+
//  The following provides for C++ compatibility (ie, C++ routines can   |
//  explicitly use this include file).                                   |
//-----------------------------------------------------------------------+
#if defined (__cplusplus)
extern "C" {
#endif


// Struct to hold the file descriptor and plot ID value for a single
// GNUplot data plot.
typedef struct {
  FILE    *gp_engine;
  int     window_idx;
  Window  win;
} GP_PLOT_EPLOT_PAIR;

  

#if defined (__cplusplus)
}
#endif


#endif // CBIC_TYPES_H
