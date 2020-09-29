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

/*//70m moving mode
#define SAMPLES 600
float x_inner = 0.08;
float y_inner = 0.08;
float x_outer = 10;
float y_outer = 10;
int steps = 2;
*/

//2km Link
#define SAMPLES 1000
float x_inner = 0.25;
float y_inner = 0.25;
float x_outer = 10;
float y_outer = 10;
int steps = 2;

int actuate(int channel, float error)
{
    int dir = 0;
    if (channel != 1 && channel != 2)
        return 0;
    if (error == 0)
        return 0;
    if (channel == 1)
        dir = 1;
    if (channel == 2)
        dir = -1;

    {
        unsigned long curr_pos = get_Position(channel);

        if (error > 0)
        {
            curr_pos += steps;
            dir = dir * 1;
        }

        else
        {
            curr_pos -= steps;
            dir = dir * 2;
        }

        go_to(channel, lu_to_string(curr_pos), false);
        return dir;
    }
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
    }

    else if (channel == 1)
    {
        if (fabs(y_avg) < y_inner)
            r = 0;
        else if (fabs(y_avg) < y_outer)
            r = actuate(1, y_avg);
    }
    return r;
}

int PID_controller(float error_X, float error_Y)
{
    int status = 0;
    static float Xsamples[SAMPLES], Ysamples[SAMPLES];
    static int index;

    //Xsamples[index] = error_X;
    //Ysamples[index] = error_Y;
    /*Code below implemented to allow operation from kinesis HV cubes.*/
    /*The lines change the (0,1) output of the HV cubes to (-1,1) as expected*/
    /*the red pitaya outputs. Original code commented above.*/
    Xsamples[index] = (error_X - 5.0) / 5.0;
    Ysamples[index] = (error_Y - 5.0) / 5.0;

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
    status = check_Avg(2);
    if (status != 0)
    {
        memset(Xsamples, 0, sizeof(Xsamples));
        return status;
    }
    status = check_Avg(1);
    if (status != 0)
    {
        memset(Ysamples, 0, sizeof(Ysamples));
        return status;
    }
    return 0;
}
