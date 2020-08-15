/*--------------------------------------------------------------
* EQ8 - SlowFeedback
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 26/05/2020
* Last modifiied on: 15/08/2020
* Version  2.4
*-------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "redpitaya/rp.h"

float mean(float list[], int n)
{
    // Return the mean of list of n numbers.
    float sum = 0.0;
    for (int loop = 0; loop < n; loop++)
    {
        sum += list[loop];
    }
    return sum / n;
}

float get_Fast_Analog(int channel, int samples)
{
    uint32_t buff_size = samples;
    float *buff = (float *)malloc(buff_size * sizeof(float));
    if (channel == 1)
        rp_AcqGetOldestDataV(RP_CH_1, &buff_size, buff);

    else if (channel == 2)
        rp_AcqGetOldestDataV(RP_CH_2, &buff_size, buff);
   
    float avg = mean(buff, samples);
    free(buff);
    return avg;
}

float get_Pin_Voltage(int pin)
{
    float value1;
    // Measure slow analogue input voltage.
    rp_AIpinGetValue(pin, &value1);
    // printf("%f\n", buff[1]);
    return value1;
}

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

float std(float array[], int n)
{
    // Calculate the standard deviation of a list of n numbers.
    float av = mean(array, 30000);
    float diff = 0.0;
    for (int loop = 0; loop < n; loop++)
    {
        diff += (av - array[loop]) * (av - array[loop]);
    }
    return diff / 29000;
}

float readSum()
{
    float value1;
    // Measure slow analogue input voltage.
    rp_AIpinGetValue(0, &value1);
    // printf("%f\n", buff[1]);
    return (value1) * (20 / 7);
}

float voltageLimiter(float *uY, float limit)
{
    if (*uY > 0.0 && *uY > limit)
        *uY = limit;

    if (*uY < 0.0 && *uY < (-1.0 * limit))
        *uY = (-1.0 * limit);

    return *uY;
}

double map(int value, int x, int y, int X, int Y)
{
    int range = y - x;
    int Range = Y - X;
    int n_value = value - x;
    double ratio = (double)n_value / (double)range;
    double N_Value = ratio * (double)Range;
    double voltage = N_Value + (double)X;
    return voltage;
}

int setup()
{
    if (rp_Init() != RP_OK)
    {
        fprintf(stderr, "Red Pitaya API init failed!\n");
        return EXIT_FAILURE;
    }
    rp_AcqSetGain(RP_CH_1, RP_HIGH);
    rp_AcqSetGain(RP_CH_2, RP_HIGH);
    rp_AcqStart();
    return EXIT_SUCCESS;
}