#ifndef CBIC_PARAMS_H
#define CBIC_PARAMS_H
/*!
*-----------------------------------------------------------------------+
* File         :  cbic_params.h                                         |
*                                                                       |
* Description  :  Main parameter header for the CBIC program.           |
*                                                                       |
* Author       :  M.Rendina                                             |
*-----------------------------------------------------------------------+*/

//-----------------------------------------------------------------------+
//  The following provides for C++ compatibility (ie, C++ routines can   |
//  explicitly use this include file).                                   |
//-----------------------------------------------------------------------+
#if defined (__cplusplus)
extern "C" {
#endif



// Maximum number of concurrent plots that can be generated.
#define GP_PLOT_MAX_ENGINES    100


//-------------------------------------
// Application version constants
//-------------------------------------
#define CBIC_APPLICATION_MAJOR_VERSION     1
#define CBIC_APPLICATION_MINOR_VERSION     10
#define CBIC_APPLICATION_PATCHLEVEL        4
#define CBIC_APPLICATION_VERSION_MODIFIER  ""


//-----------------------------------------------------------------------
// Hardcoded application configuration file name.  All other filenames
// needed by the application can be configured within the file 
// indicated here.
//-----------------------------------------------------------------------
#define APP_CONFIG_FILENAME        "cbic.conf"



#if defined (__cplusplus)
}
#endif


#endif //  CBIC_PARAMS_H
