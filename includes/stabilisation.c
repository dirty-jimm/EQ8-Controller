#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "redpitaya/rp.h"

/*float mean(float list[], int n)
{
    // Return the mean of list of n numbers.
    float sum = 0.0;
    for (int loop = 0; loop < n; loop++)
    {
        sum += list[loop];
    }
    return sum / n;
}*/

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

float readIn1()
{
    //Measure input voltage at fast analogue input 1.
    uint32_t buff_size = 10;
    float *buff = (float *)malloc(buff_size * sizeof(float));
    rp_AcqGetOldestDataV(RP_CH_1, &buff_size, buff);
    //printf("%f", buff[1]);
    free(buff);
    return mean(buff, 10);
}

float readIn2()
{
    //Measure input voltage at fast analogue input 2.
    uint32_t buff_size = 10;
    float *buff = (float *)malloc(buff_size * sizeof(float));
    rp_AcqGetOldestDataV(RP_CH_2, &buff_size, buff);
    // printf("%f\n", buff[1]);
    free(buff);
    return mean(buff, 10);
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

int stabilisation()
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

    // SETTING UP CONTROLLER VARIABLES
    //Initialise variables.
    float setPoint = 0.0;
    int iterations = 1000000, dropOut = 0.0;

    // Initialise errors in the X and Y directions.
    float e1Y = 0.0, e2Y = 0.0, e3Y = 0.0, controlVoltage1 = 0.0;
    float e1X = 0.0, e2X = 0.0, e3X = 0.0, controlVoltage2 = 0.0;

    // Initialise A, B and C coefficients in PID controller.
    float A = 0.0071;
    float B = 0.0071;
    float C = 0.0091;

    // Set voltages to (0,0).
    writeOut1(0.0);
    writeOut2(0.0);

    //    FILE *data;
    //    data = fopen("data.csv","w");

    clock_t start = clock();

    for (int i = 0; i < iterations; i++)
    {

        //    while (((clock() - start)/CLOCKS_PER_SEC) < 3600) {

        if (readSum() > 0.4)
        {

            e1Y = setPoint - (readIn1() / readSum());
            float uY = (A * e1Y) + (B * e2Y) + (C * e3Y);
            e2Y = e1Y;
            e3Y = e2Y;
            controlVoltage1 += uY;
            writeOut1(controlVoltage1);

            //          sleep(1);

            e1X = setPoint - (readIn2() / readSum());
            float uX = (A * e1X) + (B * e2X) + (C * e3X);
            e2X = e1X;
            e3X = e2X;
            controlVoltage2 += uX;
            writeOut2(controlVoltage2);

            //	    printf("ErrorX: %.4f, uX: %.6f, ControlX: %.4f, readSum: %.4f \n", e1Y, uY, controlVoltage1, readSum());
        }
        else
        {

            dropOut += 1;
            controlVoltage1 = (controlVoltage1 + 0.0) / 2.0;
            controlVoltage2 = (controlVoltage2 + 0.0) / 2.0;

            writeOut1(controlVoltage1);
            writeOut2(controlVoltage2);

            printf("Drop out number, %i. Power: %.4f \r", dropOut, readSum());
            fflush(stdout);
        }
    }

    float diff = clock() - start;
    int msec = diff * 1000 / CLOCKS_PER_SEC;
    printf("Write time taken %d seconds %d milliseconds for %i iterations. Frequency of %d Hz", msec / 1000, msec % 1000, iterations, iterations / msec / 1000);

    //    Close data.csv.
    //    fclose(data);

    writeOut1(0.0);
    writeOut2(0.0);

    //Create structure for reading data.csv into an array for data analysis.
    //    struct my_record {
    //    float errorX;
    //    float errorY;
    //    };

    //Read data into records array.
    //    FILE* my_file = fopen("data.csv","r");
    //    struct my_record records[30000];
    //    size_t count = 0;
    //    for (; count < sizeof(records)/sizeof(records[0]); ++count)
    //    {
    //        float got = fscanf(my_file, "%f,%f", &records[count].errorX, &records[count].errorY);
    //        if (got != 2) break; // wrong number of tokens - maybe end of file
    //    }
    //    fclose(my_file);

    // printf("%.3f, %.3f",records[20000].errorX,records[20000].errorY);
    // printf("%.3f, %.3f",records[20001].errorX,records[20001].errorY);
    // Calculate the standard deviation of a list of n numbers.

    //    float sumX = 0.0;
    //    float sumY = 0.0;

    //    printf("SumX %.2f", sumX);

    //    for (int loop = 0; loop < 30000; loop++) {
    //        sumX += records[loop].errorX;

    //        printf("errorX is %.8f\n", records[loop].errorX);

    //        sumY += records[loop].errorY;
    //    }

    //    float avX = sumX / 30000;
    //    float avY = sumY / 30000;
    //    float differenceX = 0.0;
    //    float differenceY = 0.0;

    //    for (int loop = 0; loop < 30000; loop++) {
    //        differenceX += (avX - records[loop].errorX)*(avX - records[loop].errorX);
    //        differenceY += (avY - records[loop].errorY)*(avY - records[loop].errorY);
    //    }

    //    float stdX = differenceX / 30000;
    //    float stdY = differenceY / 30000;

    //    printf("sumX, sumY %.4f, %.4f\n", sumX, sumY);
    //    printf("StdX, StdY is: %.4f, %.4f\n", stdX, stdY);
return 1;
}
