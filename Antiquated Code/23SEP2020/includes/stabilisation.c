/*--------------------------------------------------------------
* EQ8 - Inititilisation
* Author: Lewis Howard
* Email:
* Created on:
* Last modifiied on: 15/08/2020 by Jim Leipold
* Version 2.4
*-------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "redpitaya/rp.h"

int stabilisation()
{
float ch1_offset = 0;
float ch2_offset = 0;

    // Initialization of API.
    if (rp_Init() != RP_OK)
    {
        fprintf(stderr, "Red Pitaya API init failed!\n");
        return EXIT_FAILURE;
    }

    while (1) 
	    {
		rp_GenGetOffset(RP_CH_1, &ch1_offset);
		rp_GenGetOffset(RP_CH_2, &ch2_offset);
           	PID_controller(ch1_offset, ch2_offset);
		usleep(1000000);
	    }
return 0;
}
