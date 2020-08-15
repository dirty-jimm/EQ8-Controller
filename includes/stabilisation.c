/*--------------------------------------------------------------
* EQ8 - Inititilisation
* Author: Lewis Howard
* Email:
* Created on:
* Last modifiied on: 14/08/2020 by Jim Leipold
* 
*-------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "redpitaya/rp.h"

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

float readLightVoltage()
{
    float value1;
    // Measure slow analogue input voltage.
    rp_AIpinGetValue(0, &value1);
    // printf("%f\n", buff[1]);
    return value1;
}

float readLightVoltage_pin(int pin)
{
    float value1;
    // Measure slow analogue input voltage.
    rp_AIpinGetValue(pin, &value1);
    // printf("%f\n", buff[1]);
    return value1;
}

float readIn1()
{
    //Measure input voltage at fast analogue input 1.
    uint32_t buff_size = 10;
    float *buff = (float *)malloc(buff_size * sizeof(float));
    rp_AcqGetOldestDataV(RP_CH_1, &buff_size, buff);
    float avg = mean(buff, 10);
    free(buff);
    return avg;
}

float readIn2()
{
    //Measure input voltage at fast analogue input 2.
    uint32_t buff_size = 10;
    float *buff = (float *)malloc(buff_size * sizeof(float));
    rp_AcqGetOldestDataV(RP_CH_2, &buff_size, buff);
    float avg = mean(buff, 10);
    free(buff);
    return avg;
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

float voltageLimiter(float *uY, float limit)
{
    if (*uY > 0.0 && *uY > limit)
    {
        *uY = limit;
    }

    if (*uY < 0.0 && *uY < (-1.0 * limit))
    {
        *uY = (-1.0 * limit);
    }

    return *uY;
}

int stabilisation(int minutes)
{
    printf("\n Connecting with Red Pitaya ... \n");

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

    /* Warming up piezo's */
    printf("\n Warming up piezo mirrors ... \n");
    writeOut1(1.0);
    writeOut2(1.0);
    writeOut1(-1.0);
    writeOut2(-1.0);
    writeOut1(0.5);
    writeOut2(0.5);
    writeOut1(-0.5);
    writeOut2(-0.5);

    // SETTING UP CONTROLLER VARIABLES
    /* PID setpoint */
    float setPoint = 0.0;
    /* PID discrete error terms and output coltages */
    float e1Y = 0.0, e2Y = 0.0, e3Y = 0.0, controlVoltage1 = 0.0;
    float e1X = 0.0, e2X = 0.0, e3X = 0.0, controlVoltage2 = 0.0;
    /* PID discrete term coefficients */
    float A = 0.008; //0.008
    float B = 0.008; //0.008
    float C = 0.008; //0.0079
                     // Set voltages to (0,0).
    writeOut1(0.0);
    writeOut2(0.0);
    // QPD offsets. downstairs lab. lvl 5 lab
    float xOffset = -0.176; //0.32;//0.14502;
    float yOffset = 0.461;  //-0.26;//-0.13523;
    float sumOffset = 0.45; //0.33;//0.15926;
    // Creating file for data storage and analysis.
    FILE *data;
    data = fopen("data.csv", "w+");

    FILE *Jimsdata;
    Jimsdata = fopen("Jimsdata.csv", "w+");
    fprintf(Jimsdata, "Time, Power, Move status\n");
    // Starting the clock for timing.
    clock_t start = clock();
    float diff, sec = 0.0;
    //int minutes = atoi(argv[1]);
    int iterations = minutes * (1 * 57);
    int iterationsPerRecord = 1;
    int dropOut = 0;

    // Run a sine curve two minutes on both mirrors.
    //for (int i = 0; i < iterations; i++)

    // Starting the stabilisation loop.
    printf("\n Starting stabilisation ... \n");
    for (int i = 0; i < iterations; i++)
    {
        int moveStatus = 0;

        if (readSum() > (7.0 - sumOffset))
        {

            e1Y = (setPoint - (yOffset / sumOffset)) - ((readIn1() / readSum()) - (yOffset / sumOffset));
            float uY = (A * e1Y) + (B * e2Y) + (C * e3Y);
            e2Y = e1Y;
            e3Y = e2Y;

            //voltageLimiter(&uY, 0.2);

            if (abs(uY) > 0.2)
            {
                uY = uY / 4.0;
            }
            else
            {
                controlVoltage1 += uY;
            }

            writeOut1(controlVoltage1);

            //e1X = (setPoint - (xOffset / sumOffset)) - ((readIn2() - (xOffset)) / (readSum() - sumOffset));
            e1X = (setPoint - (xOffset / sumOffset)) - ((readIn2() / readSum()) - (xOffset / sumOffset));
            float uX = (A * e1X) + (B * e2X) + (C * e3X);
            e2X = e1X;
            e3X = e2X;

            if (abs(uX) > 0.2)
            {
                uX = uX / 4.0;
            }
            else
            {
                controlVoltage2 += uX;
            }

            writeOut2(controlVoltage2);

            if (i % iterationsPerRecord == 0)
            {

                diff = clock() - start;
                sec = diff / CLOCKS_PER_SEC;

                // float  printTimeStart = clock();
                fprintf(data, "%.5f, %.5f\n", sec, readLightVoltage());
                // float printTimeEnd = clock();
                // printf("Print to screen time %.9f", (printTimeEnd - printTimeStart)/CLOCKS_PER_SEC);
            }

            //printf("uX: %.6f, uY: %.6f, ControlX: %.4f, ControlY: %.4f, readSum: %.4f \n", uX, uY, controlVoltage2, controlVoltage1, readSum());
            moveStatus = PID_controller(controlVoltage2, controlVoltage1);
        }
        else
        {

            dropOut += 1;
            // Slewing the TT mirror to the average of current position and origin after dropout.
            controlVoltage1 -= (controlVoltage1 + 0.0) / 3.0;
            controlVoltage2 -= (controlVoltage2 + 0.0) / 3.0;

            writeOut1(controlVoltage1);
            writeOut2(controlVoltage2);

            if (i % iterationsPerRecord == 0)
            {

                diff = clock() - start;
                sec = diff / CLOCKS_PER_SEC;
                fprintf(data, "%.5f, %.5f\n", sec, readLightVoltage());
            }

            //	    printf("Drop out number, %i. Power: %.4f \n", dropOut, readSum());
        }
        fprintf(Jimsdata, "%f, %f, %i\n", sec, readLightVoltage_pin(1), moveStatus);
    }

    diff = clock() - start;
    sec = diff * 1000 / CLOCKS_PER_SEC;
    printf("\n Operational time of %.5f seconds for %i iterations. Frequency of %.5f Hz \n", sec / 1000, iterations, iterations / (sec / 1000));
    printf("\n %i dropouts recorded.\n", dropOut);
    // Close data.csv
    fclose(data);

    // Reset tip-tilt voltages to zero-point.
    writeOut1(0.0);
    writeOut2(0.0);

    //Create structure for reading data.csv into an array for data analysis.
    struct my_record
    {
        float timeSeconds;
        float fibreVoltage;
        float controlVoltage1;
        float controlVoltage2;
    };

    //Read data into records array.
    FILE *my_file = fopen("data.csv", "r");
    struct my_record records[iterations / iterationsPerRecord];
    size_t count = 0;
    for (; count < sizeof(records) / sizeof(records[0]); ++count)
    {
        float got = fscanf(my_file, "%f,%f,%f,%f", &records[count].timeSeconds, &records[count].fibreVoltage, &records[count].controlVoltage1, &records[count].controlVoltage2);
        if (got != 4)
            break; // wrong number of tokens - maybe end of file
    }

    fclose(my_file);

    float sumFibreVoltage = 0.0;
    //    float sumY = 0.0;

    for (int loop = 0; loop < (iterations / iterationsPerRecord); loop++)
    {
        sumFibreVoltage += records[loop].fibreVoltage;

        //        printf("fibreVoltage is %.8f\n", records[loop].fibreVoltage);
    }

    float avFibreVoltage = sumFibreVoltage / (iterations / iterationsPerRecord);
    float differenceFibreVoltage = 0.0;

    for (int loop = 0; loop < (iterations / iterationsPerRecord); loop++)
    {
        differenceFibreVoltage += (avFibreVoltage - records[loop].fibreVoltage) * (avFibreVoltage - records[loop].fibreVoltage);
    }

    float stdFibreVoltage = differenceFibreVoltage / (iterations / iterationsPerRecord);

    printf("Mean fibre voltage was: %.4f.\n", avFibreVoltage);
    printf("Standard deviation of fibre voltage was: %.4f.\n", stdFibreVoltage);
    printf("\a\a");
    return 1;
}
