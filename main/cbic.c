//-----------------------------------------------------------------------+
// File         :  cbic.c                                                |
//                                                                       |
// Description  :  Program to control and manage multiple accelerator    |
//                 beam measurement instruments.                         |
//                                                                       |
// Author       :  M. Rendina                                            |
//-----------------------------------------------------------------------+


#include <gtk/gtk.h>

// Core instrumentation library
//-------------------------------------
//#define CBI_CO_NO_EXTERN
//#define NO_EXTERN
#include "cbi_core_includes.h"


// Platform [BPM, TigerSHARC-based (PT)]
//---------------------------------------
//#define CBPM_NO_EXTERN
#include "cbpm_includes.h"


// CBIC (Application)
//-------------------------------------
#define CBIC_NO_EXTERN    
#include "ext_structs.h"   
#include "protos.h"
#include "cbic_params.h"
#include "faccess.h"

//=======================================================================================
//=======================================================================================
//=======================================================================================
int main( int argc, char * argv[] ) {
  
  //--------------------------
  // Install signal handlers
  //--------------------------
  // Keyboard interrupt signal handling
  if (signal (SIGINT, std_sigint_handler) == SIG_IGN) {
    signal (SIGINT, SIG_IGN);
  }
  
  // Segmentation violation handling
  if (signal (SIGSEGV, segfault_handler) == SIG_IGN) {
    signal (SIGSEGV, SIG_IGN);
  }
  
  // SIGPIPE handler - Cleans up some things if a broken pipe is encountered.
  if (signal (SIGPIPE, std_sigpipe_handler) == SIG_IGN) {
    signal (SIGPIPE, SIG_IGN);
  }


  // Pointer to name of the local function
  char *func_name = (char *)__FUNCTION__;

  char *cfg_filename = APP_CONFIG_FILENAME;

  int use_active_list = CBI_FALSE;
  int status;
  int i, j, k;
  int input_val;
  
  int BPM_IDX;  // Single number used to reference entries in the array that
                // holds pointers to each entire complement
                // of nested BPM data structures for a given instrument.  

  int commstatus;
  int iidx;

  //-----------------------------------------------------------------------+
  //                     Parse command line params here                    |
  //-----------------------------------------------------------------------+
  int  index, c;

  // Option argument defaults.
  char custom_config[50] = {""};
  int  debug_level       = 0;
  int  startup_command   = CBI_FALSE; // Whether or not to execute a command upon startup
  int  direct_to_server  = CBI_FALSE;
  int  direct_to_currmon = CBI_FALSE;
  char full_alloc_spec[51];
  char alloc_scheme[50];
  char alloc_name[50];
  
  CTL_System.RunningMode         = CBI_SINGLE_MODE;
  CTL_System.self_trigger_enable = CBI_FALSE;
  maint_mode = 0;


  // Populate the version fields of the central management structure.
  CTL_System.version.major      = CBIC_APPLICATION_MAJOR_VERSION;
  CTL_System.version.minor      = CBIC_APPLICATION_MINOR_VERSION;
  CTL_System.version.patchlevel = CBIC_APPLICATION_PATCHLEVEL;
  strcpy( CTL_System.version.modifier, CBIC_APPLICATION_VERSION_MODIFIER );


  while ((c = getopt( argc, argv, "hvf:a:bsd:gm:MTtco:pq")) != -1)
    switch (c)
      {
      case 'h':
	printf("\n\n       CESR Beam Instrumentation Control Application (CBIC)\n");
	printf("                       Version %d.%d.%d%s\n\n", 
	       CTL_System.version.major,
	       CTL_System.version.minor,
	       CTL_System.version.patchlevel,
	       CTL_System.version.modifier );
	printf(" Command line options:\n\n");

	printf("  -h    This help information.\n\n");

	printf("  -v    Version information for this application and its supporting components.\n\n");

	printf("  -f <filename>   ");
	printf("         Specify a particular application configuration file to use. \n");
	printf("         This allows for command-line requesting of different sets of \n");
	printf("         instruments to bring online in a given session and other startup \n");
	printf("         options.  The file must reside in the current working directory. \n");
	printf("         If this option is not specified, the default config file \n");
	printf("         'cbic.conf' will be read from the current working directory\n\n");

	printf("  -a <allocation-spec>  \n");
	printf("	 Specify an instrument allocation scheme / allocation name pair \n");
	printf("         to use for this application instance.\n\n");

	printf("  -b    'BACK-END' mode.  Start program, reading configuration as normal, then drop into\n");
	printf("         a wait state for an incoming network connection from a manager program, \n");
	printf("         potentially on a remote host.\n");
        printf("         Once a connection is established, the manager will have full control of this\n");
	printf("         program.\n\n");

	printf("  -s    Go directly into SERVER MODE.\n");
	printf("        Once all instruments are polled and brought \n");
	printf("        online, this will place the application into BPM data server mode to allow \n");
	printf("        3rd party beam position requests.  CESRV and orbit are two programs that use \n");
	printf("        a BPM server to acquire their data.\n\n");

	printf("  -c    Go directly into CURRENT MONITOR MODE\n");
	printf("        Intended for bringing a SINGLE instrument online and activating \n");
	printf("        it to allow continuous bunch-by-bunch current monitoring\n");
	printf("        The instrument to use for monitoring currents is specified in\n");
	printf("        the instrument list file with a 'CURRMON' server ID specification.\n\n");
	
	printf("  -d <#>  Set debug verbosity level 0 - %d\n", CBI_NUM_DEBUG_LEVELS-1);
	printf("          Determines the level of detail that debugging and trace printouts\n");
	printf("          show during operation. \n");
	printf("                 0 means no additional debugging output- equivalent to not using '-d' \n");
	printf("                 5 means maximum detail in debugging output\n");
	printf("          The application defaults to level 0 if no option is specified.\n");
	printf("          NOTE: Level 5 prints a LOT of information and may in fact clutter useful \n");
	printf("                information or scroll it off the screen more rapidly than it may be \n");
        printf("                productively used.  It is recommended that the application is run \n");
	printf("                in a terminal window with a large scrollback buffer so that runtime \n");
	printf("                details are not lost.\n\n");
	
	printf("  -m <#>  Enter 'MAINTENANCE MODE' wherein instruments are brought online with a \n");
	printf("          customizable set of communication checks and parameter reads.\n");
	printf("                 0 means no maintenance mode will be used- equivalent to not using '-m' \n");
	printf("                 1 \n");
	printf("                 2 \n");
	printf("                 3 Highest supported maintenance mode.  Reads in general parameters but \n");
	printf("                   avoids all communication with instruments to allow low level tasks to be \n");
	printf("                   performed.  Example: To perform a rapid blanket reset/wakeup of all \n");
	printf("                   instruments after a power cycle of several instruments has taken place, \n");
	printf("                   start up application in maintenance mode 3 and then run the RESET INSTRUMENT\n");
	printf("                   command.  Wait about 20 seconds.  All instruments should be ready for use.\n");
	printf("                   Exit and start application normally.\n\n");
	
	printf("  -M    Enter 'MANAGER MODE' wherein this application will start (remote) instances of\n");
	printf("        itself in Back-end mode and then connect to them for centralized control.\n\n");
	
	printf("  -T    TESTING MANAGER MODE. The same as -M above, but DO NOT spawn copies of each back-end \n");
	printf("        instance requested.  The back-ends will need to be started manually.  Used for testing.\n\n");

	printf("  -t    Trigger-enable mode.  The program will set the trigger (data) enable flag in the \n");
        printf("        CESR database on its own once all instruments are set up and ready to acquire data.\n");
	printf("        The program then de-asserts the trigger enable once the measurement is complete.\n\n");
	
	printf("  -g    Graphical User Interface (GUI) mode.  Allows for windowed, graphical\n");
	printf("        control of some program functions.\n\n");
	printf("  -o <#>  Enter Debug Option mode 1 choice\n\n");
	printf("  -p <#>  Enable Debug Option mode 2 choice\n\n");
	printf("  -q <#>  Enable Debug Option mode 3 choice\n\n");
	exit(0);
	break;

      case 'v':
	printf("\n\n        CESR Beam Instrumenation Control Application (CBIC)\n");
	printf("                       Version %d.%d.%d%s\n", 
	       CBIC_APPLICATION_MAJOR_VERSION,
	       CBIC_APPLICATION_MINOR_VERSION,
	       CBIC_APPLICATION_PATCHLEVEL,
	       CBIC_APPLICATION_VERSION_MODIFIER );
	printf("                         GCC %d.%d.%d\n\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__ );
	printf("-----------------------------------------------------------------------\n");
	printf("           Core (CO) communication structure set version : %10d\n", CBI_COMM_STRUCT_REV);
	printf("-----------------------------------------------------------------------\n");
	printf(" TigerSHARC BPM (PT) communication structure set version : %10d\n", CBPM_COMM_STRUCT_REV);
	printf("-----------------------------------------------------------------------\n");
	printf("\n\n");
	exit(0);
	break;

      case 'f':
	if (strlen(optarg) > 50 ) {
	  printf("Custom application config file name is too long.\n");
	  printf("Please rename the file to fewer than 50 characters\n");
	  printf("and try again.\n");
	  exit(1);
	}
	strcpy( custom_config, optarg );
	break;

      case 'a':
	if (strlen(optarg) > 50) {
	  printf("Instrument allocation identifier is too long.\n");
	  printf("Please ensure that the identifier is correct and try again.\n");
	  exit(1);
	}
	char temp[50]  = "";
	strcpy( full_alloc_spec, optarg );
	strcpy( alloc_scheme, strtok( full_alloc_spec, ":" ) );
	strcpy( alloc_name, strtok( NULL, ":" ) );
	if ( strlen(alloc_scheme) == 0 || strlen(alloc_name) == 0 ) {
	  printf("Problem with allocation spec.  Examine it and try again.\n");
	}
	break;

      case 'b':
	printf("Entering BACK-END mode...\n");
	CTL_System.RunningMode = CBI_BACKEND_MODE;
	printf("RunningMode = %d\n", CTL_System.RunningMode);
	break;


      case 's':
	startup_command  = CBI_TRUE;
	direct_to_server = CBI_TRUE;
	break;

      case 'd':
	if ( strlen(optarg) > 1 ) {
	  printf("Invalid argument to option '-d'\n");
	  return 1;
	}
	debug_level = atoi(optarg);
	if (debug_level < 0 || debug_level > CBI_NUM_DEBUG_LEVELS) {
	  printf("Debug printout level requested is not in supported range.\n");
	  return 1;
	}
	CTL_System.debug_verbosity = debug_level;
	printf("DEBUG PRINTOUT level %d requested.\n", debug_level);
	break;

      case 'g':
	use_gui = TRUE;
	printf("GUI MODE\n");
	break;

      case 'm':
	if ( strlen(optarg) > 1 ) {
	  printf("Invalid argument to option '-m'\n");
	  return 1;
	}
	maint_mode = atoi(optarg);
	if (maint_mode < 0 || maint_mode > 3) {
	  printf("Maintenance mode level requested is not in supported range.\n");
	  return 1;
	}
	printf("MAINTENANCE MODE level %d requested.\n", maint_mode);
	CTL_System.maintenance_mode = maint_mode;
	break;

      case 'M':
	CTL_System.RunningMode = CBI_MANAGER_MODE;
	printf("\n\n\nMANAGER MODE\n\n\n");
	break;

      case 'T':
	CTL_System.RunningMode = CBI_MGR_TESTING_MODE;
	printf("\n\n\nMANAGER TESTING MODE\n");
	printf("You must start each back-end instance manually on\n");
	printf("the appropriate hosts.\n\n");
	break;

      case 't':
	CTL_System.self_trigger_enable = CBI_TRUE;
	printf("\n\n NOTE: Running in SELF-TRIGGERED mode.\n");
	printf("       Application will set the CBPM COMMAND data enable word\n");
	printf("       on its own to start acquisitions and reset it once the\n");
	printf("       acquisition is complete.\n\n");
	break;

      case 'c':
	startup_command     = CBI_TRUE;
	cbpm_currmon_server = CBI_TRUE;
	direct_to_currmon   = CBI_TRUE;
	printf("CURRENT MONITOR MODE\n");
	break;
	
      case 'o':
	if ( strlen(optarg) > 1 ) {
	  printf("Invalid argument to option '-o'\n");
	  return 1;
	}
	option_mode = atoi(optarg);
	printf("\tOPTION MODE: %d enabled.\n", option_mode);
	CTL_System.option_mode = option_mode;	  
	break;

      case 'p':
	printf("\t\t\tOPTION MODE2 enabled.\n");
	option_mode2 = 1;
	CTL_System.option_mode2 = option_mode2;	  
	break;

      case 'q':
	printf("\t\t\t\t\tOPTION MODE3 enabled.\n");
	option_mode3 = 1;
	CTL_System.option_mode3 = option_mode3;	  
	break;

      case '?':
	if (optopt == 'x')
	  fprintf(stderr, "Option -%c requires an argument.\n", optopt);
	else if (isprint (optopt))
	  fprintf(stderr, "Unknown option `-%c'.\n", optopt);
	else
	  fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
	return 1;
      default:
	abort();
      }
  
  printf("debug level = %d, gui flag = %d, maint mode = %d \n", 
         debug_level, 
         use_gui, 
         maint_mode );
  
  for (index = optind; index < argc; index++) {
     printf("Non-option argument %s\n", argv[index]);
     return 0;
  }
  
  
  
  // Initialize the GTK+ GUI code
  if( gtk_init_check(&argc, &argv) ) {
     printf("\nGTK+ Initialization successful.\n\n");
     gui_capable = 1;
  } else {
     printf("\nGTK+ Initialization FAILED!\n");
     printf(" GUI functionality is NOT available.\n\n");
  }
  
   

  //--------------------------------------------------------------------------------
  // TODO: Based on instrument type BPM, BSM, etc, load up the number of available
  //       commands and attach the command list itself to the central list pointer.
  //--------------------------------------------------------------------------------
  CTL_System.itype.num_platform_commands = cbpm_num_commands;
  CTL_System.command_list                = (CBI_COMMAND *)&(cbpm_commands);
  
  // Assign menu numbers to available commands.
  // TODO: Push numbering and menu mask honoring to menu display function.
  for (i = 0; i < CTL_System.itype.num_platform_commands; i++) {
    CTL_System.command_list[i].menuID = i;
  }

  // TODO: Condense all platform-specific initializations to one instance here.
  

  //----------------------------------------------------------------------
  // Attach supplementary instrument information output function here for 
  // more detail in online instrument listing.
  //----------------------------------------------------------------------
  // Pre-load with NULL, then provide selection logic based on instrument type.
  CTL_System.itype.tsetup_func_ptr    = NULL;
  CTL_System.itype.supp_info_func_ptr = NULL;

  //
  ////CTL_System.supp_info_func_ptr = &(cbpm_print_inst_info);




  //-------------------------------------------
  // Memory structure setup and initialization
  //-------------------------------------------
  cbi_mem_setup( );

  // TODO: Add instrument platform selection here
  cbpm_mem_setup( );




  //---------------------------------------------------------------------
  // Read in the application configuration file
  //---------------------------------------------------------------------
  char *conf_filename;
  if ( strlen(custom_config) > 1 ) {
    printf("Using custom application config file './%s'\n", custom_config);
    conf_filename = (char *)custom_config;
  } else {
    printf("Using default application config file './cbic.conf'.\n");
    conf_filename = (char *)APP_CONFIG_FILENAME;
  }
  fflush(stdout);


  printf("Reading application configuration...\n");
  status = cbi_read_config_file( conf_filename );
  if (status != CBI_F_SUCCESS) {
     printf("Configuration file parse error.  Please check the formatting\n");
     printf("of the file '%s' before attempting to continue.  Exiting.\n", conf_filename);
     exit(1);
  }
  fflush(stdout);


  //------------------------------------------------------------
  // If started as part of a two-tier arrangement, initialize
  // two-tier communications channels and handle tier-specific
  // startup tasks.
  //------------------------------------------------------------
  if (CTL_System.RunningMode == CBI_MANAGER_MODE || 
      CTL_System.RunningMode == CBI_MGR_TESTING_MODE ||
      CTL_System.RunningMode  == CBI_BACKEND_MODE       ) {
    //two_tier_startup();
    two_tier_startup( alloc_scheme );

  }



  //---------------------------------------------------------------------------
  // Examine configuration paramters and assign final values after performing
  // any necessary error checking.
  //---------------------------------------------------------------------------

  // FIXME: TODO: Manage number of timing setups for dissimilar platforms.
  for (i = 0; i < CBPM_MAX_TIMING_SETUPS; i++) {
    if ( strcmp( appconfig.system_timing_setup_label, tsetup_names[i]) == 0 ) {
      appconfig.system_timing_setup = i;
      printf("Timing setup taken from config file: (%d) %s \n", i, tsetup_names[i] );
      break;
    }
    if (i == CBPM_MAX_TIMING_SETUPS-1) {
      printf("Requested timing setup %s does not map to a valid\n", appconfig.system_timing_setup_label );
      printf("timing setup value.  Check the startup configuration file and try again.\n");
      return F_FAILURE;
    }
  }

  if ( appconfig.default_gain_setting < 0 || appconfig.default_gain_setting > CBPM_MAX_GAIN_SETTING) {
    appconfig.default_gain_setting = 1;
    printf(" !- Default gain setting requires: An integer in the range 0 - %d\n", CBPM_MAX_GAIN_SETTING );
    printf("    Setting to a value of '1' and continuing.\n");
  } 

  
  // Set the default timing setup value here.
  int tsetup = appconfig.system_timing_setup;
  
  
  // Initialize the MPMnet connection if the XBUS communications
  // method has been specified.
  status = cbi_init_mpm_comms();
  if (status == CBI_F_FAILURE) {
     printf("\n\nMPMnet/XBUS Communications initialization failure!\n");
     printf(" MPMnet communications method is not available.\n\n");
     ////exit(2);
  }
  fflush(stdout);

    
   CBPM_DATA         *dp, *tdp; // Pointers to data substructure trees
   CBPM_CONTROL      *cp;       // Pointer to control substructure tree

  if ( CTL_System.RunningMode != CBI_MANAGER_MODE ) {

    // NEW: Read in a particular instrument allocation
    //-------------------------
    CTL_System.n_Modules = cbi_read_allocation(appconfig.inst_alloc_file,
					       alloc_scheme,
					       alloc_name   );
    printf("\n\nNumber of instruments requested = %d\n", CTL_System.n_Modules);



    //---------------------------------------------------------------------
    // Read in operational parameters for each instrument verfied as being
    // online and associate the assigned detectors with each unit.
    //    Read in and store separate timing values for each available
    //    gain regime.  
    //---------------------------------------------------------------------
    printf("\n\n\n----------------------------------------------------------------------\n");
    printf("Reading general instrument parameters for all requested instruments...\n");
    printf("----------------------------------------------------------------------\n");
    // FIXME: TODO: Allow selection of instrument-specific file reading routine here
    cbpm_read_detector_params( appconfig.det_param_file );
    fflush(stdout);


    printf("\n\n\n----------------------------------------------------------------------\n");
    printf("Reading instrument-specific parameters for all requested instruments...\n");
    printf("----------------------------------------------------------------------\n");
    // FIXME: TODO: Allow selection of instrument-specific file reading routine here
    cbpm_read_instrument_params( appconfig.inst_param_file );
    fflush(stdout);

    
    printf("\n\n\n----------------------------------------------------------------------\n");
    printf("Loading the Instrument Code parameters to all requested instruments...\n");
    printf("----------------------------------------------------------------------\n");
    cbpm_load_instr_code_params();
    fflush(stdout);    


    //----------------------------------------------
    // Verify connectivity and identification here
    //----------------------------------------------
    verify_connectivity();



    if (CTL_System.n_Modules == 1) {
      printf("\n\n 1 Instrument is ONLINE.\n\n");
    } else {
      printf("\n\n %d Instruments are ONLINE.\n\n", CTL_System.n_Modules );
    }
    

    cbi_open_sockets_active();


    //----------------------------------------------------
    // Attach platform-specific instrument executable 
    // routine name list to master management structure.  
    // TODO: Generalize based on desired instrument type/platform.
    //----------------------------------------------------
    //CTL_System.itype.instrument_routine_names = (char *)&( cbpm_routine_names[0] );



    //----------------------------------------------------
    // Set up and activate all the instruments requested
    //----------------------------------------------------
    char ldr_name[20];
    
    
    double start;
    //---------------------------------------------------
    // Second pass through all online instruments to 
    // configure everything else, prepare, and activate
    // each instrument.
    //---------------------------------------------------
    int integrity_good;
    int species;

    if (CTL_System.maintenance_mode < 3) {

      for (BPM_IDX = 0; BPM_IDX < CTL_System.n_Modules; BPM_IDX++) {

	// Specify not ready for database update (Necessary?)
	CTL_System.p_Module[BPM_IDX]->db_update = NO_UPDATE;
	// Initialize recovery flag.
	CTL_System.p_Module[BPM_IDX]->in_recovery = CBI_FALSE;

	if (CTL_System.p_Module[BPM_IDX]->active != CBI_MAINT_1 &&
	    CTL_System.p_Module[BPM_IDX]->active != CBI_MAINT_2 &&
	    CTL_System.p_Module[BPM_IDX]->active != CBI_MAINT_3 &&
	    CTL_System.p_Module[BPM_IDX]->active != CBI_MAINT_4 )   {
	
	  dp  = CTL_System.p_Module[BPM_IDX]->dsp_data;
	  cp  = CTL_System.p_Module[BPM_IDX]->control;
	  
	  printf("Location     = %s\n", CTL_System.p_Module[BPM_IDX]->det.location );	
	  
	  printf("Checking communications structure versions for instrument <%s>...\n", 
		 CTL_System.p_Module[BPM_IDX]->comm.ethernet.hostname );
	  
	  // Read all the core configuration info from the instrument
	  k=cbi_gp_struct(READ, CTL_System.comm_method, CBI_MODULE_CONFIG_TAG, CTL_System.p_Module[BPM_IDX], CBI_FULL_STRUCT);
	  
	  // Verify all tiers of communication structure revision numbers
	  if (CTL_System.p_Module[BPM_IDX]->core->cbi_module_config.core_comm_struct_rev != CBI_COMM_STRUCT_REV) {
	    printf("ERROR!  The CORE communication structure version of the instrument\n");
	    printf("        does not match the version used to build the control application!\n");
	    printf("          APPLICATION VERSION         INSTRUMENT VERSION\n");
	    printf("            %010d                         %010d \n",
		   CBI_COMM_STRUCT_REV,
		   CTL_System.p_Module[BPM_IDX]->core->cbi_module_config.core_comm_struct_rev);
	    printf("        \nTo avoid this error, ensure that the control application uses the same\n");
	    printf("        communication structure header files as the instrument's executable.\n\n\n");
	    //CTL_System.p_Module[BPM_IDX]->active = CBI_MAINT_1;
	    CTL_System.p_Module[BPM_IDX]->active = CBI_INACTIVE;
	  }
	  if (CTL_System.p_Module[BPM_IDX]->core->cbi_module_config.platform_comm_struct_rev != CBPM_COMM_STRUCT_REV) {
	    printf("ERROR!  The PLATFORM communication structure version of the instrument\n");
	    printf("        does not match the version used to build the control application!\n");
	    printf("          APPLICATION VERSION         INSTRUMENT VERSION\n");
	    printf("            %010d                         %010d \n",
		   CBPM_COMM_STRUCT_REV,
		   CTL_System.p_Module[BPM_IDX]->core->cbi_module_config.platform_comm_struct_rev);
	    printf("        \nTo avoid this error, ensure that the control application uses the same\n");
	    printf("        communication structure header files as the instrument's executable.\n\n\n");
	    //CTL_System.p_Module[BPM_IDX]->active = CBI_MAINT_1;
	    CTL_System.p_Module[BPM_IDX]->active = CBI_INACTIVE;
	  }
	  
	    
	  // Verify instrument EXE name?
	  printf("EXE Name = %s\n", CTL_System.p_Module[BPM_IDX]->core->cbi_module_config.ldr_name );

	  if (CTL_System.p_Module[BPM_IDX]->active == CBI_ACTIVE) {
	  
	    printf("Setting up and activating BPM  %s (%-15s)  @  %s %d \n",
		   CTL_System.p_Module[BPM_IDX]->comm.ethernet.hostname,
		   CTL_System.p_Module[BPM_IDX]->comm.ethernet.IP_address,
		   CTL_System.p_Module[BPM_IDX]->comm.xbus.pkt_node,
		   CTL_System.p_Module[BPM_IDX]->comm.xbus.element );
	    
	    
	    // Load active timing setup specification into communication structure
	    dp->cbpm_op_timing.active_timing_setup = tsetup;

	    dp->cbpm_op_gain.active_gain_settings[0][0] = CBI_UNINITIALIZED_PARAM;
	    printf("Initializing active gain setting to '%d'\n", CBI_UNINITIALIZED_PARAM );
	    
	    
	    // Determine if the instrument's heartbeat is active before
	    // dispatching any setup command(s).
	    if ( cbi_check_heartbeat(BPM_IDX) ) {
	      
	      //---------------------------------------------------------------------
	      // Populate and send phase configuration structure
	      //---------------------------------------------------------------------
	      for (species = 0; species < CBI_NUM_SPECIES; species++) {
		dp->cbpm_phase_config.phase_jump_boundaries[species][0] = cp->config_params.phase_jump_boundaries[species][0];
		dp->cbpm_phase_config.phase_turn_offsets[species][0] = cp->config_params.phase_turn_offsets[species][0];
		dp->cbpm_phase_config.phase_turn_offsets[species][1] = cp->config_params.phase_turn_offsets[species][1];
		dp->cbpm_phase_config.phase_wait_values[species][0] = cp->config_params.phase_wait_values[species][0];
		dp->cbpm_phase_config.phase_wait_values[species][1] = cp->config_params.phase_wait_values[species][1];
	      }
	      printf("Sending all: phase, ");
	      k=cbi_gp_struct(WRITE, CTL_System.comm_method, CBPM_PHASE_CONFIG_TAG, CTL_System.p_Module[BPM_IDX], CBI_FULL_STRUCT);
	      
	      
	      //---------------------------------------------------------------------
	      // Send gain configuration structure
	      //---------------------------------------------------------------------
	      printf("pedestal, ");
	      k=cbi_gp_struct(WRITE, CTL_System.comm_method, CBPM_PEDESTAL_CONFIG_TAG, CTL_System.p_Module[BPM_IDX], CBI_FULL_STRUCT);
	      
	      printf("gain configuration, ");
	      status = cbi_gp_struct(WRITE, CTL_System.comm_method, CBPM_GAIN_CONFIG_TAG, CTL_System.p_Module[BPM_IDX], CBI_FULL_STRUCT);
	      
	      printf("and timing configuration settings. \n");
	      status = cbi_gp_struct(WRITE, CTL_System.comm_method, CBPM_TIMING_CONFIG_TAG, CTL_System.p_Module[BPM_IDX], CBI_FULL_STRUCT);
	      
	      dp->cbpm_op_timing.active_timing_setup = appconfig.system_timing_setup;
	      
	      
	      printf("Commanding gain and timing setup settings...\n");
	      k=cbi_gp_struct(WRITE, CTL_System.comm_method, CBPM_OP_TIMING_TAG, CTL_System.p_Module[BPM_IDX], CBI_FULL_STRUCT);
	      cbi_init_command( BPM_IDX, SET_TIMING_SETUP_CMD );
	      cbi_execute_command( BPM_IDX );
	      cbi_wait_for_command( BPM_IDX );
	      // FIXME: Generalize
	      cbpm_set_timing_setup_post(BPM_IDX); // Ensures sync with proper gain setting for timing setup requested.

	      printf("Sending all code parameters...\n");
	      k=cbi_gp_struct(WRITE, CTL_System.comm_method, CBPM_CODE_PARAMS_TAG, CTL_System.p_Module[BPM_IDX], CBI_FULL_STRUCT);
	      

	      int tries;
	      integrity_good = FALSE;
	      // Try up to 10 times to get a good timing integrity check
	      // and wait 0.1s inbetween each try.
	      // Otherwise, the delay in execution here has some effect on
	      // whether or not a good timing integrity value is read back.
	      // Uncertain as to why this is just now.  FIXME.
	      for (tries = 0; tries < 10; tries++) {
		if ( cbi_check_timing_integrity(BPM_IDX) ) {
		  integrity_good = TRUE;
		  break;
		}
		cbi_sleep(100);
	      }
	      
	      if (integrity_good) {
		
		CTL_System.p_Module[BPM_IDX]->active = CBI_ACTIVE;
	      } else {
		CTL_System.p_Module[BPM_IDX]->active = CBI_INACTIVE;
	      }
	      
	    } else {
	      printf("Instrument heartbeat not present!  Instrument is online, but has been set to inactive.\n");
	      CTL_System.p_Module[BPM_IDX]->active = CBI_INACTIVE;
	    }
	  
	    printf("Final timing setup = %d [%s]\n", 
		   dp->cbpm_op_timing.active_timing_setup, 
		   tsetup_names[dp->cbpm_op_timing.active_timing_setup] );
	    
	    printf("\n\n\n");
	    fflush(stdout);
	    
	  }  else {
	   
	    printf("Skipping initialization of instrument due to version mismatch.\n");
	    
	  }//endIF active == CBI_ACTIVE


	} // endIF active == CBI_MAINT_...
	
	
      } // end BPM_IDX for loop
      
    }  else { // If in the highest supported maintenance mode (finalize details), skip all communication steps
              // and enable every instrument by default so that explicitly toggling their activity state is not
              // required once the application is up.  Allows for maintenance tasks to be performed immediately
              // upon startup.
      
      for (BPM_IDX = 0; BPM_IDX < CTL_System.n_Modules; BPM_IDX++) {
	CTL_System.p_Module[BPM_IDX]->active = CBI_ACTIVE;
      }

    } //endIF (CTL_System.maintenance_mode < 3)
    
    

  } // endIF CTL_System.RunningMode (!= MANAGER_MODE)
  

  cbi_close_sockets();



  int  command;
  int  num_args;
  int  use_enable_gate;
  
  // Function pointer definitions for use below.
  int (*fptr1)()   = NULL;
  int (*fptr2)()   = NULL; // For '_post()' function calls.



  int seq_step, sequence_steps;
  int num_sequence_steps;
  int command_index = -1;

  CTL_System.cmd_dispatch_method = CBI_LOCAL_CMD_DISPATCH;



  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // User input polling loop
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  while(TRUE) {
    
    fptr1 = fptr2 = NULL;
    
    //------------------------------------------------
    // Wipe all READ/WRITE communications structures 
    // in the temporary struct tree.
    //------------------------------------------------
    cbi_wipe_comm_structs( CTL_System.p_Module_temp, TOT_CBPM_COMM_STRUCTS );
    
    
    use_enable_gate = CBI_FALSE;
    
    //---------------------------------------------------------------
    // If setup for a local command dispatch, generate the menu and 
    // prompt for input.  Otherwise, use the appropriate method to 
    // obtain the command to execute.
    //---------------------------------------------------------------
    if ( CTL_System.RunningMode == CBI_MANAGER_MODE ) {
      cbi_manager_input_handler( CTL_System.command_list, CTL_System.itype.num_platform_commands );
    }

    if (startup_command) {
      
      // One clause for each unique startup command
      if (direct_to_server) {
	printf("Proceeding directly into SERVER MODE as per command line request\n");
	command_index = cbi_command_idx_from_name( CTL_System.command_list, "ENTER SERVER MODE" );
	CTL_System.skip_comm_connect = TRUE;
	direct_to_server = CBI_FALSE;
      }

      if (direct_to_currmon) {
	printf("Proceeding directly into 4ns CURRENT MONITOR MODE as per command line request\n");
	command_index = cbi_command_idx_from_name( CTL_System.command_list, "MEASURE 4ns BUNCH CURRENTS" );
	direct_to_currmon = CBI_FALSE;
      }

      // Common to all startup commands
      CTL_System.cmd_dispatch_method = CBI_LOCAL_CMD_DISPATCH;
      startup_command                = CBI_FALSE;

      
    } else if ( CTL_System.cmd_dispatch_method == CBI_LOCAL_CMD_DISPATCH ) {
      
      
      input_val = cbi_query_action( CTL_System.command_list, CTL_System.itype.num_platform_commands );
      if (debug_level > CBI_DEBUG_LEVEL_2) {
	printf("input_val = %d\n", input_val);
      }
      
      
      // Search available commands for the one requested by user
      for (command = 0; command < CTL_System.itype.num_platform_commands; command++) {
	if ( CTL_System.command_list[command].menuID == input_val ) {
	  command_index = command;
	}
      }
      
      if (command_index == -1) {
	printf("Invalid menu item\n");
	continue;
      }

      if ((debug_level > CBI_DEBUG_LEVEL_0) && (input_val >= 0)) {
        printf("\n\nExecuting command: %i - %s\n\n", input_val, CTL_System.command_list[input_val].name);
      }
      
    } // else, this is a SELF command dispatch for recovery purposes.


    
    //------------------------------------------------------------
    // If command is a sequence, load the sequence and loop over
    // steps, executing separate preps and posts, if requested, 
    // across all instruments for each step in the sequence.
    //------------------------------------------------------------
    CTL_System.num_sequence_commands = 0;
    
    if (CTL_System.command_list[command_index].type == SEQUENCE_TYPE) {
       // Reset number of sequence commands in the queue
       fptr1 = CTL_System.command_list[command_index].prep_func;

       num_sequence_steps = (*fptr1)();
       printf("Sequence type encountered with %d steps.\n", num_sequence_steps);
    } else {
      cbi_load_command( cbpm_commands,
		    CTL_System.itype.num_platform_commands,
		    cbpm_commands[command_index].name,
		    CBI_TRUE );
      num_sequence_steps = 1;
    }
    


    cbi_dispatch_sequencer( CTL_System.command_list, command_index, TOT_CBPM_COMM_STRUCTS, num_sequence_steps );



    cbi_inst_status_handler( CTL_System.command_list, &command_index );

    
  } // endWHILE(true)
  
  
} // end main
