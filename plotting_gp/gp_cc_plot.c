/*!-----------------------------------------------------------------------+
* File         :  gp_plot.c                                               |
*                                                                         |
* Description  :  Generic plotter of data                                 |
*                 Assumes gnuplot is available in path.                   |
*                                                                         |
*                                                                         |
* Arguments    :  datfile - Pointer to string holding datafile name       |
*                 col_start - First column of data (start numbering at 1) |
*                 col_end - Last column of data                           |
*                 plot_title - String holding the desired plot title      |
*                 titles - Array of strings holding titles for each data  |
*                   series to plot.  (FIXME: add some error checking here) |
*                                                                         |
* Author       :  M.Rendina                                               |
*                                                                         |
*-------------------------------------------------------------------------+*/

/* #include "standard_headers.h" */


/* int gp_cc_plot( int BPM_IDX, char *datfile, int dir, int col_start, int col_end, char *plot_title, char titles[][MAX_STRING_LENGTH] ) { */

/*   int i=0; */
/*   int col; */
/*   FILE *plotscript;   */

/*   p_tsharc_ctl_mod = &(tsharc_ctl_mod[BPM_IDX]); // pointer for passing BPM structs to comm functions */

/*   CBI_PT_DATA  *dptr; */
/*   dptr = p_tsharc_ctl_mod->dsp_data; */

/*   char funcs[4][6] = {"F0(x)", "F1(x)", "F2(x)", "F3(x)"}; */

/*   // Write gnuplot command file */
/*   plotscript = fopen("plotcc.gp", "w"); */
  
/*   // Compose plot appearance commands */
/*   //fprintf(plotscript, "set xtics 1\n"); */
/*   fprintf(plotscript, "set xtics %d\n", pparams.major_ticks); */
/*   fprintf(plotscript, "set grid\n\n"); */
/*   fprintf(plotscript, "set style data points\n"); */
/*   fprintf(plotscript, "set title \"%s\"  \n", plot_title); */

/*   fprintf(plotscript, "set arrow from %f,305 to %f,675 nohead lt 1 lw 1.2\n", supp_plotvals[0], supp_plotvals[0] ); */
/*   fprintf(plotscript, "set arrow from %f,295 to %f,675 nohead lt 2 lw 1.2\n", supp_plotvals[1], supp_plotvals[1] ); */
/*   fprintf(plotscript, "set arrow from %f,285 to %f,675 nohead lt 3 lw 1.2\n", supp_plotvals[2], supp_plotvals[2] ); */
/*   fprintf(plotscript, "set arrow from %f,275 to %f,675 nohead lt 4 lw 1.2\n", supp_plotvals[3], supp_plotvals[3] ); */

/*   fprintf(plotscript, "set arrow from %f,305 to %f,675 nohead lt 1 lw 1.2\n", supp_plotvals[4], supp_plotvals[4] ); */
/*   fprintf(plotscript, "set arrow from %f,295 to %f,675 nohead lt 2 lw 1.2\n", supp_plotvals[5], supp_plotvals[5] ); */
/*   fprintf(plotscript, "set arrow from %f,285 to %f,675 nohead lt 3 lw 1.2\n", supp_plotvals[6], supp_plotvals[6] ); */
/*   fprintf(plotscript, "set arrow from %f,275 to %f,675 nohead lt 4 lw 1.2\n", supp_plotvals[7], supp_plotvals[7] ); */

/*   fprintf(plotscript, "F0(x) = a0*x**2 + a1*x + a2 \n"); */
/*   fprintf(plotscript, "F1(x) = b0*x**2 + b1*x + b2 \n"); */
/*   fprintf(plotscript, "F2(x) = c0*x**2 + c1*x + c2 \n"); */
/*   fprintf(plotscript, "F3(x) = d0*x**2 + d1*x + d2 \n"); */

/*   fprintf(plotscript, "a0 = %f\n", dptr->cbi_PT_find_delay_out.fcoeffs[0]); */
/*   fprintf(plotscript, "a1 = %f\n", dptr->cbi_PT_find_delay_out.fcoeffs[1]); */
/*   fprintf(plotscript, "a2 = %f\n", dptr->cbi_PT_find_delay_out.fcoeffs[2]); */

/*   fprintf(plotscript, "b0 = %f\n", dptr->cbi_PT_find_delay_out.fcoeffs[3]); */
/*   fprintf(plotscript, "b1 = %f\n", dptr->cbi_PT_find_delay_out.fcoeffs[4]); */
/*   fprintf(plotscript, "b2 = %f\n", dptr->cbi_PT_find_delay_out.fcoeffs[5]); */

/*   fprintf(plotscript, "c0 = %f\n", dptr->cbi_PT_find_delay_out.fcoeffs[6]); */
/*   fprintf(plotscript, "c1 = %f\n", dptr->cbi_PT_find_delay_out.fcoeffs[7]); */
/*   fprintf(plotscript, "c2 = %f\n", dptr->cbi_PT_find_delay_out.fcoeffs[8]); */

/*   fprintf(plotscript, "d0 = %f\n", dptr->cbi_PT_find_delay_out.fcoeffs[9]); */
/*   fprintf(plotscript, "d1 = %f\n", dptr->cbi_PT_find_delay_out.fcoeffs[10]); */
/*   fprintf(plotscript, "d2 = %f\n", dptr->cbi_PT_find_delay_out.fcoeffs[11]); */

/*   // Create full file path from generated name and */
/*   // data output path gotten from app configruation info. */
/*   //------------------------------------------------------ */
/*   char full_fname[MAX_STRING_LENGTH]; */
/*   *full_fname = (int)NULL; // Initialize as empty string */
/*   strcat( full_fname, appconfig.directories[dir] ); */
/*   strcat( full_fname, "/" ); */
/*   strcat( full_fname, datfile ); */

/*   fprintf(plotscript, "plot '%s' ", full_fname); */

/*   // Compose actual plot command */
/*   int numseries = col_end - col_start; */
/*   for (i=0; i < numseries; i++ ) { */
/*     col = col_start + i + 1; */

/*     if (i == 0) {  // First series */
/*       if (col == col_end ) { // This is the first AND the last column to plot */
/* 	fprintf(plotscript, "using 1:%d title \"%s\" lt %d, %s lt %d \n", col, titles[i], i+1, funcs[i], i+1 ); */
/*       } else { */
/* 	fprintf(plotscript, "using 1:%d title \"%s\" lt %d, %s lt %d, \\\n", col, titles[i], i+1, funcs[i], i+1 ); */
/*       } */
/*       continue; */
/*     } */

/*     if (col == col_end ) {  // Last series */
/*       fprintf(plotscript, "'' using 1:%d title \"%s\" lt %d, %s lt %d\n", col, titles[i], i+1, funcs[i], i+1 ); */
/*     } else { */
/*       fprintf(plotscript, "'' using 1:%d title \"%s\" lt %d, %s lt %d,\\\n", col, titles[i], i+1, funcs[i], i+1 ); */
/*     } */

/*   } */
  
  


/*   fprintf(plotscript, "pause -1\n"); */

/*   fclose(plotscript); */

/*   // Generate the plot */
/*   system("xterm -e gnuplot plotcc.gp &"); // allows multiple plots at the same time */
/*   //system("gnuplot plot.gp &"); */

/*   return F_SUCCESS; */

/* } */





