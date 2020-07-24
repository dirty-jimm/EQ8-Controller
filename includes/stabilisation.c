#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "redpitaya/rp.h"

float mean (float list[], int n) {
    // Return the mean of list of n numbers.
    float sum = 0.0;
    for (int loop = 0; loop < n; loop++) {
        sum += list[loop];
    }
    return sum / n;
}

float std (float array[], int n) {
    // Calculate the standard deviation of a list of n numbers.
    float av = mean(array, 30000);
    float diff = 0.0;
    for (int loop = 0; loop < n; loop++) {
        diff += (av - array[loop])*(av - array[loop]);
    }
    return diff / 29000;
}

float readSum () {
    float value1;
    // Measure slow analogue input voltage.
    rp_AIpinGetValue(0, &value1);
    // printf("%f\n", buff[1]);
    return (value1) * (20/7);
}

float readLightVoltage () {
    float value1;
    // Measure slow analogue input voltage.
    rp_AIpinGetValue(0, &value1);
    // printf("%f\n", buff[1]);
    return value1;
}

float readIn1 () {
    //Measure input voltage at fast analogue input 1.
    uint32_t buff_size = 10;
    float *buff = (float *)malloc(buff_size * sizeof(float));
    rp_AcqGetOldestDataV(RP_CH_1, &buff_size, buff);
    float avg = mean(buff, 10);
    free(buff);
    return avg;;
}

float readIn2 () {
    //Measure input voltage at fast analogue input 2.
    uint32_t buff_size = 10;
    float *buff = (float *)malloc(buff_size * sizeof(float));
    rp_AcqGetOldestDataV(RP_CH_2, &buff_size, buff);
    float avg = mean(buff, 10);
    free(buff);
    return avg;
}

int writeOut1 (float OFS) {
    /* Generating offset */
    rp_GenOffset(RP_CH_1, OFS);
    /* Enable channel */
    rp_GenOutEnable(RP_CH_1);
    return 0;
}

int writeOut2 (float OFS) {
    /* Generating offset */
    rp_GenOffset(RP_CH_2, OFS);
    /* Enable channel */
    rp_GenOutEnable(RP_CH_2);
    return 0;
}

int main () {
    // Initialization of API.
    if (rp_Init() != RP_OK) {
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
    // Initialise errors in the X and Y directions.
    float e1Y = 0.0, e2Y = 0.0, e3Y = 0.0, controlVoltage1 = 0.0;
    float e1X = 0.0, e2X = 0.0, e3X = 0.0, controlVoltage2 = 0.0;
    // Initialise A, B and C coefficients in PID controller.
    float A = 0.0061;
    float B = 0.0061;
    float C = 0.0081;
    // Set voltages to (0,0).
    writeOut1(0.0);
    writeOut2(0.0);
    // QPD offsets.
    float xOffset = 0.14502;
    float yOffset = -0.13523;
    float sumOffset = 0.15926;
    // Creating file for data storage and analysis.
    FILE *data;
    data = fopen("data.csv","w");
    // Starting the clock for timing.
    clock_t start = clock();
    float diff, sec = 0.0;
    int iterations = 200000, dropOut = 0;

    // Starting the stabilisation loop.
    for (int i=0; i<iterations; i++) {

//    while (((clock() - start)/CLOCKS_PER_SEC) < 3600) {

        if (readSum() > (0.35 - sumOffset)) {

            e1Y = setPoint - ((readIn1() - (yOffset)) / (readSum() - sumOffset));
	    float uY  = (A * e1Y) + (B * e2Y) + (C * e3Y);
	    e2Y = e1Y;
            e3Y = e2Y;
            controlVoltage1 += uY;
            writeOut1(controlVoltage1);

	    e1X = setPoint - ((readIn2() - (xOffset)) / (readSum() - sumOffset));
            float uX  = (A * e1X) + (B * e2X) + (C * e3X);
            e2X = e1X;
            e3X = e2X;
            controlVoltage2 += uX;
            writeOut2(controlVoltage2);

	    if (i % 100 == 0) {

		diff = clock() - start;
		sec = diff / CLOCKS_PER_SEC;

		// float  printTimeStart = clock();
                fprintf(data,"%.5f, %.5f, %.4f, %.4f\n", sec, readLightVoltage(), controlVoltage1, controlVoltage2);
		// float printTimeEnd = clock();
		// printf("Print to screen time %.9f", (printTimeEnd - printTimeStart)/CLOCKS_PER_SEC);

            }

	    // printf("ErrorX: %.4f, uX: %.6f, ControlX: %.4f, readSum: %.4f \n", e1Y, uY, controlVoltage1, readSum());

	// sleep(0.001);


	} else {

            dropOut +=  1;
            // Slewing the TT mirror to the average of current position and origin after dropout.
            controlVoltage1 -= (controlVoltage1 + 0.0)/3.0;
            controlVoltage2 -= (controlVoltage2 + 0.0)/3.0;

            writeOut1(controlVoltage1);
            writeOut2(controlVoltage2);

            if (i % 100 == 0) {

		diff = clock() - start;
		sec = diff / CLOCKS_PER_SEC;
                fprintf(data,"%.5f, %.5f, %.4f, %.4f\n", sec, readLightVoltage(), controlVoltage1, controlVoltage2);

            }

	    printf("Drop out number, %i. Power: %.4f \n", dropOut, readSum());

       }

    }

    diff = clock() - start;
    sec = diff * 1000 / CLOCKS_PER_SEC;
    printf("Operational time of %.5f seconds for %i iterations. Frequency of %.5f Hz \n", sec/1000, iterations, iterations/(sec / 1000));

    // close data.csv
    fclose(data);

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

}

