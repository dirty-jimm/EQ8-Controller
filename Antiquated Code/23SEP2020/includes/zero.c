/*--------------------------------------------------------------
* EQ8 - Inititilisation
* Author: Lewis Howard
* Email:
* Created on:
* Last modifiied on: 6/08/2020 by Jim Leipold
* 
*-------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "redpitaya/rp.h"

int writeOut1(float OFS)
{
    /* Generating offset */
    rp_GenOffset(RP_CH_1, OFS);
    /* Enable channel */
    rp_GenOutEnable(RP_CH_1);
    return 0;
}

int writeOut2(float OFS)
{
    /* Generating offset */
    rp_GenOffset(RP_CH_2, OFS);
    /* Enable channel */
    rp_GenOutEnable(RP_CH_2);
    return 0;
}

int main(int argc, char **argv)
{
    // Initialization of API.
    if (rp_Init() != RP_OK)
    {
        fprintf(stderr, "Red Pitaya API init failed!\n");
        return EXIT_FAILURE;
    }

    // SETTING UP FAST ANALOGUE OUTPUTS.
    // Set both fast analogue inputs to the high voltage setting.
    rp_AcqSetGain(RP_CH_1, RP_HIGH);
    rp_AcqSetGain(RP_CH_2, RP_HIGH);
    /* Generating frequency */
    rp_GenFreq(RP_CH_1, 0.0);
    /* Generating amplitude */
    rp_GenAmp(RP_CH_1, 0.0);
    /* Generating frequency */
    rp_GenFreq(RP_CH_2, 0.0);
    /* Generating amplitude */
    rp_GenAmp(RP_CH_2, 0.0);
    /* Generating wave form */
    rp_GenWaveform(RP_CH_1, RP_WAVEFORM_DC);
    /* Generating wave form */
    rp_GenWaveform(RP_CH_2, RP_WAVEFORM_DC);

    writeOut1(0);
    writeOut2(0);
}
    