#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int get_Analog(int channel)
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
}

double map(int value, int x, int y, int X, int Y)
{
    int range = y-x;
    int Range = Y-X;
    int n_value = value - x;
    double ratio = (double)n_value/(double)range;
    double N_Value = ratio * (double)Range;
    double voltage = N_Value + (double)X;
    return voltage;
}


