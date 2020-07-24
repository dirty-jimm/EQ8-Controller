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
#define K_i 0.000005
#define OUTER_CLIPPING 0.05
#define INNER_CLIPPING 5

bool calibrated = false;
float set_point_X = 0, set_point_Y = 0;

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
            printf("Calibrating: %i%%\r", i*100/samples);
            fflush(stdout);
        }
        set_point_X = set_point_X/samples;
        set_point_Y = set_point_Y/samples;

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

int PID_controller()
{
    if (calibrated == false)
        calibrate();

    time_t prev = time(NULL);
    float X_curr = 0, Y_curr = 0;
    static int X_prev, Y_prev;
    double xi = 0.0, yi = 0.0;

    while (true)
    {
        X_curr = get_Analog(1, 1000) - set_point_X;
        Y_curr = get_Analog(2, 1000) - set_point_Y;
        printf("Xcurr: %7.4f, Ycurr: %7.4f, ", X_curr, Y_curr);

        xi = xi + K_i * ((X_curr + X_prev) / 2);
        yi = yi + K_i * ((Y_curr + Y_prev) / 2);

        if (xi > 0 && xi > OUTER_CLIPPING)
            xi = OUTER_CLIPPING;

        else if (xi < 0 && xi < -OUTER_CLIPPING)
            xi = -OUTER_CLIPPING;

        if (yi > 0 && yi > OUTER_CLIPPING)
            yi = OUTER_CLIPPING;

        else if (yi < 0 && yi < -OUTER_CLIPPING)
            yi = -OUTER_CLIPPING;

        X_prev = X_curr;
        Y_prev = Y_curr;

        printf("xi %7.4f, yi %7.4f\r", xi, yi);
        fflush(stdout);

        if (time(NULL) - prev >= 5)
        {
            prev = time(NULL);
            //if (xi != 0)
            //turn(2, xi);
        }
    }
}
