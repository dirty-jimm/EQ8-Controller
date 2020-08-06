/*--------------------------------------------------------------
* EQ8 - SlowFeedback
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 26/05/2020
* Last modifiied on: 21/07/2020
* 
* This library contains high level mount controlling functionality:
*   
*-------------------------------------------------------------*/
#include <time.h>

#define VERSION_SLOW_FEEDBACK 2.14
#define K_i 0.0000005
#define OUTER_CLIPPING 1
#define INNER_CLIPPING 0
#define SAMPLES 100

bool calibrated = false;
float set_point_X = 0, set_point_Y = 0;
float x_avg, y_avg;

void calibrate()
{
    char input[MAX_INPUT];
    printf("Run calibration y/n?: ");
    scanf("%s", input);
    if (strcasecmp(input, "y") == 0 || strcasecmp(input, "yes"))
    {
        const int samples = 1000;
        for (int i = 0; i < samples; i++)
        {
            set_point_X += get_Analog(1, 16000);
            set_point_Y += get_Analog(2, 16000);
            printf("Calibrating: %i%%\r", i * 100 / samples);
            fflush(stdout);
        }
        set_point_X = set_point_X / samples;
        set_point_Y = set_point_Y / samples;

        printf("X set point %f, Y set point %f\n", set_point_X, set_point_Y);
    }
}

int actuate(int channel, int error)
{
    if (channel != 1 && channel != 2)
        return -1;
    else if (error == 0)
        return 0;
    else
        turn(channel, 1);
    return 1;
}

void PID_controller(float error_X, float error_Y)
{
    static int X_prev, Y_prev;
    static double xi, yi;
    static int i;

    //X_curr = error_x; //get_Analog(1, 1000) - set_point_X;
    //Y_curr = error_y; //get_Analog(2, 1000) - set_point_Y;
    xi = xi + K_i * ((error_X + X_prev) / 2);
    yi = yi + K_i * ((error_Y + Y_prev) / 2);

    if (xi > 0 && xi > OUTER_CLIPPING)
        xi = OUTER_CLIPPING;

    else if (xi < 0 && xi < -OUTER_CLIPPING)
        xi = -OUTER_CLIPPING;

    if (yi > 0 && yi > OUTER_CLIPPING)
        yi = OUTER_CLIPPING;

    else if (yi < 0 && yi < -OUTER_CLIPPING)
        yi = -OUTER_CLIPPING;

    X_prev = error_X;
    Y_prev = error_Y;

    printf("error_X: %7.4f, error_Y: %7.4f, xi: %7.4f, yi: %7.4f, i: %i \r", error_X, error_Y, xi, yi, i);
    fflush(stdout);

    i++;
    if (i > 10000)
    {
        i = 0;
        // actuate(1, xi);
        // actuate(2, yi);
    }
}

int PID_controller2(float error_X, float error_Y)
{
    static int Xsamples[SAMPLES], Ysamples[SAMPLES];
    static int index;

    Xsamples[index] = error_X;
    Ysamples[index] = error_Y;
    index++ % SAMPLES;

    
    //printf("error_X: %7.4f, error_Y: %7.4f, xi: %7.4f, yi: %7.4f, i: %i \r", error_X, error_Y, xi, yi, i);
    fflush(stdout);

    i++;
    if (i > 10000)
    {
        i = 0;
        // actuate(1, xi);
        // actuate(2, yi);
        return 1;
    }
}
