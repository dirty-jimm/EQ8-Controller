#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "redpitaya/rp.h"

/*int get_Analog(int channel)
{
    FILE *fp = popen("acquire 1", "r");
    char response[32];
    if (fp == NULL)
    {
        printf("Failed to run command\n");
        exit(1);
    }
    while (fgets(response, sizeof(response), fp) != NULL)
    {
        //printf("Response:%s\n", response);
    }

int value = atoi(strtok(response, " "));
if(channel == 2)
    value = atoi(strtok(NULL, " "));

pclose(fp);
    return value;
}*/

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

float get_Analog(int channel, int samples)
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