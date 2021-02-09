/*!-----------------------------------------------------------------------+
* File         :  two_tier_startup.c                                      |
*                                                                         |
* Description  :  Sets up socket communications for talking to multiple   |
*                 back-end processes from a single manager instance.      |
*                 Controls spawning and placement of subprocess windows,  |
*                 the startup options for each back-end subprocess, and   |
*                 confirmation of successful back-end start up.           |
*                                                                         |
* Arguments    :                                                          |
*                                                                         |
*         TODO: Cleanup constants
*               Refine window spawn / positioning procedure
*               Allow passing of user-selected back-end general config
*                  file name
*               Allow runtime selection of testing state
*               Allow pass-through of debug mode, other cmd-line requests
*               Standardize handshaking string constants and place 
*                  definitions in  header file
*
* Author       :  M. Rendina                                              |
*-------------------------------------------------------------------------+*/

#include "standard_headers.h"

/* The port to which the manager will be connecting */
// Allow fail-over and force-retry socket option to provide
// rapid turnaround in the event quick restarts are performed?
#define PORT 8001

  
  /* how many pending connections queue will hold */
#define BACKLOG 10

  // max number of bytes we can get at once
#define MAXDATASIZE 300

#define QUERY_BACKEND            "BACK-END?"
#define BACKEND_ACK              "BACK-END reporting for duty!"
#define EXPECTED_CONNECTIONS     2

#define TESTING_OPERATION        0              // 1 = TRUE,   0 = FALSE
//#define OPERATIONAL_LOCATION     CONTROL_ROOM
#define OPERATIONAL_LOCATION     OFFICE_TEST



//int two_tier_startup( void ) {
int two_tier_startup( char *alloc_scheme ) {

  printf("Allocation scheme = [%s]\n", alloc_scheme);

  int rmode = CTL_System.RunningMode;
  
  int sockfd;
  int tsockfd;
  char buf[MAXDATASIZE];    
  struct sockaddr_in my_addr;
  // connector’s address information
  struct sockaddr_in their_addr;
  int sin_size;
  struct sigaction sa;
  int yes = 1;


  // Client-specific (talker) new connection on new_fd 
  struct hostent *he;
  
  // Manager-specific (Listener)
  int numbytes;
  int new_fd;
  int connection;

  enum OP_LOCATION {
    CONTROL_ROOM,
    OFFICE_TEST,
    NUM_OP_LOCATIONS
  };


  CTL_System.num_mgmt_connections = EXPECTED_CONNECTIONS;

  // TODO: Roll this up into menu-based or config-file based option specification.

  //ar backend_hosts[2][10] = {"lnx184c", "lnx185c"};
  char backend_hosts[2][10];
  //char locations[2][13]     = {"WEST", "EAST"};
  char config_files[2][2][35]  = {"cbic.conf-14ns-FixedGain", "cbic.conf-14ns-FixedGain",
                                  "cbic.conf-TEST-1",              "cbic.conf-TEST-2" };


/*   char allocation_names[2][2][35] = {"Custom:test1", "Custom:test2", */
/*   				     "Custom:test1", "Custom:test2" }; */
/*   strcpy( mgrconfig.alloc_schemes[0], "Custom"); */
/*   strcpy( mgrconfig.alloc_schemes[1], "Custom"); */
/*   strcpy( mgrconfig.alloc_names[0], "test1"); */
/*   strcpy( mgrconfig.alloc_names[1], "test2"); */


  //char allocation_names[2][2][35] = {"Standard:West", "Standard:East",
  //			     "Standard:West", "Standard:East" };
  //strcpy( mgrconfig.alloc_schemes[0], "Standard");
  //strcpy( mgrconfig.alloc_schemes[1], "Standard");
  //strcpy( mgrconfig.alloc_names[0], "West");
  //strcpy( mgrconfig.alloc_names[1], "East");


  strcpy( mgrconfig.alloc_schemes[0], alloc_scheme );
  strcpy( mgrconfig.alloc_schemes[1], alloc_scheme );


  // Obtain each allocation name from the provided scheme
  int  retcode = 0;
  int  element = 1;
  int  instcount = 0;
  char name_token[10] = {""};
  char instring[50];


  FILE *fp, *fpm;
  char error[200];
  fp = fopen(appconfig.inst_alloc_file, "r");
  if (fp == NULL) {
    sprintf(error, "Error opening allocation file %s ", appconfig.inst_alloc_file);
    perror( error );
    return CBI_F_FAILURE;
  }

  while( retcode == 0 ) {
    sprintf( name_token, "NAME_%d", element );
    element++;
    retcode = faccess( FA_READ, fp, alloc_scheme, name_token, 1, 1, STRING, instring );
    if ( retcode == 0 ) {
      strcpy( mgrconfig.alloc_names[instcount], instring );
      instcount++;
    } else {
      printf("Stopped collecting allocation names from the scheme.\n");
    }
  }
  rewind(fp);
  


  printf("test\n");
  strcpy( instring, "" );
  retcode = 0;
  element = 1;
  instcount = 0;

  // Obtain backend host for each instance
  while( retcode == 0 ) {
    sprintf( name_token, "HOST_%d", element );
    element++;
    retcode = faccess( FA_READ, fp, alloc_scheme, name_token, 1, 1, STRING, instring );
    if ( retcode == 0 ) {
      strcpy( backend_hosts[instcount], instring );
      instcount++;
    } else {
      printf("Stopped collecting host names from the scheme.\n");
    }
  }
  rewind(fp);



  int  expected_connections = EXPECTED_CONNECTIONS;
  //---------------------------------------------   X     Y
  int  window_positions[NUM_OP_LOCATIONS][2] = { 1940,    0,
					  	  940,    0, };


  char sys_string[200];
  int  attempt;



  if (rmode == CBI_MANAGER_MODE || rmode == CBI_MGR_TESTING_MODE) {
    
    // Read in allocation info and process for all requested back-ends.
    cbi_manager_read_allocations( expected_connections );      

    if (rmode == CBI_MANAGER_MODE) {

      for (connection = 0; connection < expected_connections; connection++) {
	
	// Spawn a (potentially remote) instance of CBIC as a back-end.
	/*      sprintf( sys_string, */
	/* 	       "xterm -si -T \"BPM Server %d  -  %s\" -geometry 134x70+%d+%d -e ssh %s \"cd /nfs/cesr/instr/CBPM_II_Servers && hostname && /home/mcr/devel/instr/bin/cbic -f %s -a %s -b\" &", */
	/* 	       connection+1, */
	/* 	       backend_hosts[connection], */
	/* 	       connection * window_positions[OPERATIONAL_LOCATION][0], */
	/* 	       connection * window_positions[OPERATIONAL_LOCATION][1] + 0*2105, */
	/* 	       backend_hosts[connection], */
	/* 	       config_files[TESTING_OPERATION][connection], */
	/* 	       allocation_names[TESTING_OPERATION][connection] ); */

	sprintf( sys_string,
		 "xterm -si -T \"BPM Server %d  -  %s\" -e ssh %s \"cd /nfs/cesr/instr/CBPM_II_Servers && hostname && /home/mcr/devel/instr/bin/cbic -f %s -a %s -b\" &",
		 connection+1,
		 backend_hosts[connection],
		 backend_hosts[connection],
		 config_files[TESTING_OPERATION][connection],
		 mgrconfig.alloc_names[connection] );
		 //allocation_names[TESTING_OPERATION][connection] );
	

	if (CTL_System.debug_verbosity >= CBI_DEBUG_LEVEL_1) {
	  printf("Command: %s\n", sys_string);
	}
	
	////cbi_sleep(500);
	cbi_sleep(1500);
	system( sys_string );
	
      } //endFOR connection

    } else {                                    // Testing mode just skips back-end spawning,
                                                // revert to normal MANAGER MODE at this point.
      CTL_System.RunningMode = CBI_MANAGER_MODE;
    }


    for (connection = 0; connection < expected_connections; connection++) {

      printf("***************Number of instruments [%d] = %d\n", connection, mgrconfig.num_instruments[connection] );
      
      // get the host info
      if( (he=gethostbyname(backend_hosts[connection])) == NULL) {
	perror("gethostbyname()");
	exit(1);
      } else {
	printf("Manager: The remote host is: %s\n", backend_hosts[connection] );
      }
      
      if((CTL_System.mgmt_socket_IDs[connection] = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("socket()");
	exit(1);
      } else {
	printf("Manager: The socket() sockfd is OK...\n");
      }
      
      // host byte order
      their_addr.sin_family = AF_INET;
      // short, network byte order
      printf("Manager: Connecting to %s on port %d...\n", backend_hosts[connection], PORT);
      their_addr.sin_port = htons(PORT);
      their_addr.sin_addr = *((struct in_addr *)he->h_addr);
      // zero the rest of the struct
      memset(&(their_addr.sin_zero), '\0', 8);


      for (attempt = 0; attempt < 5; attempt++) {
	cbi_sleep(1000);
	if(connect(CTL_System.mgmt_socket_IDs[connection], (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
	  perror("connect() error");
	  if (attempt == 4) {
	    printf("Final connection attempt failed, quitting.\n");
	    exit(1);
	  }
	} else {
	  printf("Manager: connect() OK...\n");
	  break;
	}
      } // endFOR


      strcpy( buf, "" );
      
      
      // Query back-end for acknowledgement of presence
      if(send(CTL_System.mgmt_socket_IDs[connection], QUERY_BACKEND, strlen(QUERY_BACKEND)+1, 0) == -1) {
	perror("send() error");
	close(CTL_System.mgmt_socket_IDs[connection]);
	exit(1);
      }

      // Wait for response
      recv(CTL_System.mgmt_socket_IDs[connection], buf, MAXDATASIZE, 0);
      if ( strcmp( buf, BACKEND_ACK ) == 0 ) {
	printf("Received IDENT string: \"%s\"\n", buf);
	printf("Done.\n");
      } else {
	printf("Received incorrect identification from back-end.  Quitting.\n");
	exit(1);
      }
      fflush(stdout);

    
      // Send assigned ID (connection/instance) number to back-end
      strcpy( buf, "" );
      sprintf( buf, "%d", connection+1 );
      printf("Assigning ID number %s...\n", buf);
      if(send(CTL_System.mgmt_socket_IDs[connection], buf, strlen(QUERY_BACKEND)+1, 0) == -1) {
	perror("send() error");
	close(CTL_System.mgmt_socket_IDs[connection]);
	exit(1);
      }


      // Send instrument quantity offset value to back-end
      int cxn;
      int quant_offset = 0;
      if (connection != 0) {
	for (cxn= 0; cxn < connection; cxn++) {
	  quant_offset += mgrconfig.num_instruments[cxn];
	}
      }
      printf("\nQUANT OFFSET = %d\n", quant_offset);
      strcpy( buf, "" );
      sprintf( buf, "%d\0", quant_offset );
      printf("Sending quant offset value %s...\n", buf);
      if(send(CTL_System.mgmt_socket_IDs[connection], buf, strlen(buf)+1, 0) == -1) {
	perror("send() error");
	close(CTL_System.mgmt_socket_IDs[connection]);
	exit(1);
      }


    } //endFOR connection



     
  } else if (rmode == CBI_BACKEND_MODE) { 
    

    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      perror("Back-end: socket() error");
      exit(1);
    } else {
      printf("Back-end: socket() sockfd is OK...\n");
      // Store socketfd centrally
      CTL_System.mgmt_socket_IDs[0] = sockfd;   // FIXME: RENAME
    }
    
    if (setsockopt(CTL_System.mgmt_socket_IDs[0], SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("Back-end: setsockopt() error");
      exit(1);
    } else {
      printf("Back-end: setsockopt is OK...\n");
    }
    
    /* host byte order */
    my_addr.sin_family = AF_INET;
    /* short, network byte order */
    my_addr.sin_port = htons(PORT);
    /* automatically fill with my IP */
    my_addr.sin_addr.s_addr = INADDR_ANY;
    
    
    printf("Server-Using %s and port %d...\n", inet_ntoa(my_addr.sin_addr), PORT);
    
    
    
    /* zero the rest of the struct */
    memset(&(my_addr.sin_zero), '\0', 8);
    
    if(bind(CTL_System.mgmt_socket_IDs[0], (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
      perror("bind() error");
      exit(1);
    } else {
      printf("bind() OK...\n");
    }
    
    if(listen(CTL_System.mgmt_socket_IDs[0], BACKLOG) == -1) {
      perror("listen() error");
      exit(1);
    }
    printf("listen() is OK...Listening...\n");
    
    
    /* accept() loop */
    sin_size = sizeof(struct sockaddr_in);
    if((CTL_System.mgmt_socket_IDs[0] = accept(CTL_System.mgmt_socket_IDs[0], (struct sockaddr *)&their_addr, &sin_size)) == -1) {
      perror("Back-end accept() error");
      exit(1);
    } else {
      printf("Back-end accept() is OK...\n");
      printf("Back-end: new socket, sockfd is OK...\n");
      printf("Back-end: Got socketfd from %s\n", inet_ntoa(their_addr.sin_addr));
    }
    
    
    // Wait for presence acknowledgement query
    recv(CTL_System.mgmt_socket_IDs[0], buf, MAXDATASIZE, 0);
    if ( strcmp( buf, QUERY_BACKEND ) == 0 ) {
      printf("Received query string: \"%s\"\n", buf);
      printf("Sending acknowledgement info...\n");
      if(send(CTL_System.mgmt_socket_IDs[0], BACKEND_ACK, strlen(BACKEND_ACK)+1, 0) == -1) {
	perror("send() error");
	close(CTL_System.mgmt_socket_IDs[0]);
	exit(1);
      }
      printf("Done.\n");
      fflush(stdout);
    } else {
      printf("Received incorrect query \"%s\".  Quitting.\n", buf);
      exit(1);
    }
    
    
    // Wait to receive identification (instance) number from manager
    strcpy( buf, "" );
    recv(CTL_System.mgmt_socket_IDs[0], buf, MAXDATASIZE, 0);
    printf("Received ident string: \"%s\"\n", buf);
    CTL_System.instance_ID = atoi(buf);
    printf("Stored ID value of %d\n",  CTL_System.instance_ID );
    fflush(stdout);
    if (CTL_System.instance_ID < 0 || CTL_System.instance_ID > 9) {
      printf("Received invalid ID value \"%s\".  Quitting.\n", buf);
      exit(1);
    }


    // Wait to receive instrument quantity offset from manager
    strcpy( buf, "" );
    recv(CTL_System.mgmt_socket_IDs[0], buf, MAXDATASIZE, 0);
    printf("Received instrument quantity offset string: \"%s\"\n", buf);
    CTL_System.inst_quant_offset = atoi(buf);
    printf("Stored instrument quantity offset value of %d\n",  CTL_System.inst_quant_offset );
    fflush(stdout);


    struct timeval tv;
    //tv.tv_sec = 0; // Never time out. (Doesn't appear to behave as expected.  i.e. Still times out.)
    tv.tv_sec = 2000000;
    setsockopt(CTL_System.mgmt_socket_IDs[0], SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv, sizeof(struct timeval));


    fflush(stdout);
    //sleep(4);
    /*     if ( CTL_System.instance_ID == 2 ) { */
    /*       printf("TEST -- Sleeping for 110 seconds to simulate many BPM initializations...\n"); */
    /*       fflush(stdout); */
    /*       sleep(110); */
    /*     } */
    /*       } else { */
    /* 	printf("TEST -- Sleeping for 30 seconds to simulate many BPM initializations...\n"); */
    /* 	fflush(stdout); */
    /* 	sleep(30); */
    /*       } */
    

  } // endIF rmode
  
  return CBI_F_SUCCESS;

}


