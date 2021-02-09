#ifndef STANDARD_HEADERS_H
#define STANDARD_HEADERS_H
//-----------------------------------------------------------------------+
// File         :  standard_headers.h                                    |
//                                                                       |
// Description  :  This holds the most commonly shared header include    |
//                 statements used in this project.  Include this        |
//                 header to take care of most required includes in one  |
//                 step.                                                 |
//                                                                       |
// Author       :  M.Rendina                                             |
//-----------------------------------------------------------------------+

//-----------------------------------------------------------------------+
// The following provides for C++ compatibility (ie, C++ routines can    |
// explicitly use this include file).                                    |
//-----------------------------------------------------------------------+
#if defined (__cplusplus)
extern "C" {
#endif



// Beam Instrumentation Core
//-------------------------------------
#include "cbi_core_includes.h"


// Instrument-platform-specific library includes (BPM-TSHARC)
//-------------------------------------
#include "cbpm_includes.h"


// Application
//-------------------------------------
#include "cbic_params.h"
#include "cbic_types.h"
#include "protos.h"      // Function prototypes for this project
#include "ext_structs.h" // Structures and variables shared across entire project



//-----------------------------------------------------------------------+
// The following provides for C++ compatibility (ie, C++ routines can    |
// explicitly use this include file).                                    |
//-----------------------------------------------------------------------+
#if defined (__cplusplus)
}
#endif

#endif // STANDARD_HEADERS_H
