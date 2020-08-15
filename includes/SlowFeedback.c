/*--------------------------------------------------------------
* EQ8 - SlowFeedback
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 26/05/2020
* Last modifiied on: 14/08/2020
* Version  2.3
* NOTES: 1 encoder step = 0.12 arc second = 5.82e-7 rad ~= 0.6 micro rad
*-------------------------------------------------------------*/

#define VERSION_SLOW_FEEDBACK 2.14

float x_avg, y_avg;

/*//70m stationary mode
#define SAMPLES 10000
float x_inner = 0.08;
float y_inner = 0.08;
float x_outer = 10;
float y_outer = 10;
int steps = 2;
*/

//70m moving mode
#define SAMPLES 600
float x_inner = 0.08;
float y_inner = 0.08;
float x_outer = 10;
float y_outer = 10;
int steps = 10;

int actuate(int channel, float error)
{

    if (channel != 1 && channel != 2)
        return -1;
    else if (error == 0)
        return 0;
    else
    {
        unsigned long curr_pos = get_Position(channel);
        if (channel == 1)
        {
            if (error > 0)
                curr_pos += steps;
            else
                curr_pos -= steps;
        }
        else
        {
            if (error > 0)
                curr_pos += steps;
            else
                curr_pos -= steps;
        }
        return go_to(channel, lu_to_string(curr_pos), false);
    }
    return 1;
}

int check_Avg(int channel)
{
    int r = 0;
    if (channel == 2)
    {
        if (fabs(x_avg) < x_inner)
            r = 0;
        else if (fabs(x_avg) < x_outer)
            r = actuate(2, x_avg);
        else
            r = -1;
    }

    else if (channel == 1)
    {
        if (fabs(y_avg) < y_inner)
            r = 0;
        else if (fabs(y_avg) < y_outer)
            r = actuate(1, y_avg);
        else
            r = -1;
    }
    return r;
}

int PID_controller(float error_X, float error_Y)
{
    int status = 0;
    static float Xsamples[SAMPLES], Ysamples[SAMPLES];
    static int index;

    Xsamples[index] = error_X;
    Ysamples[index] = error_Y;
    index++;
    index %= SAMPLES;

    x_avg = mean(Xsamples, SAMPLES);
    y_avg = mean(Ysamples, SAMPLES);
    if (verbose)
        printf("X: % 6.3f, Y: % 6.3f\n", x_avg, y_avg);
    else
    {
        printf("X: % 6.3f, Y: % 6.3f\r", x_avg, y_avg);
        fflush(stdout);
    }
    if (check_Avg(2))
    {
        memset(Xsamples, 0, sizeof(Xsamples));
        status = 1;
    }

    if (check_Avg(1))
    {
        memset(Ysamples, 0, sizeof(Ysamples));
        status = 1;
    }
    return status;;
}
