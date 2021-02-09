#ifndef EXT_STRUCTS_H
#define EXT_STRUCTS_H
//-----------------------------------------------------------------------+
// File         :  ext_structs.h                                         |
//                                                                       |
// Description  :  Header for declaring structures available across all  |
//                 source files used in the application.                 |
//                                                                       |
// Author       :  M. Rendina                                            |
//-----------------------------------------------------------------------+
#include "cbi_core_includes.h"
#include "cbic_params.h"



#if defined (CBIC_NO_EXTERN)


// File IO mutex
pthread_mutex_t mutex_fp;

int gui_capable = 0;
int use_gui     = 0;
int maint_mode  = 0;
int option_mode  = 0;
int option_mode2  = 0;
int option_mode3  = 0;


#else 


extern pthread_mutex_t mutex_fp;

extern int gui_capable;
extern int use_gui;
extern int maint_mode;


#endif // (CBIC_NO_EXTERN)



#endif // (ext_structs_H)
