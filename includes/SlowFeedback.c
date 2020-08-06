/*--------------------------------------------------------------
* EQ8 - SlowFeedback
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 26/05/2020
* Last modifiied on: 21/07/2020
*   
*-------------------------------------------------------------*/

#define VERSION_SLOW_FEEDBACK 2.14
#define SAMPLES 100
int x_inner = 1;
int y_inner = 1;
int x_outer = 2;
int y_outer = 2;
float x_avg, y_avg;

int actuate(int channel, int error)
{
    if (channel != 1 && channel != 2)
        return -1;
    else if (error == 0)
        return 0;
    else
        {   
        unsigned long curr_pos = get_Position(channel);
        if(error>0)
            curr_pos++;
        else
            curr_pos--;
           
        return go_to(channel, lu_to_string(curr_pos), false);
        }
    return 1;
}

int check_Avg(int channel)
{
    if (channel == 1 || channel == 3)
    {
        if (abs(x_avg) < x_inner)
         {}
        else if (abs(x_avg) < x_outer)
        return actuate(1, x_avg);
    }

    if (channel == 2 || channel == 3)
    {
        if (abs(y_avg) < y_inner)
            return 0;
        else if (abs(y_avg) < y_outer)
            return actuate(2, y_avg);
    }
    return 2;
}

int PID_controller(float error_X, float error_Y)
{
    static float Xsamples[SAMPLES], Ysamples[SAMPLES];
    static int index;

    Xsamples[index] = error_X;
    Ysamples[index] = error_Y;
    index++;
    index %= SAMPLES;

    x_avg = mean(Xsamples, SAMPLES);
    y_avg = mean(Ysamples, SAMPLES);
    printf("X: %06.3f, Y: %06.3f\r", x_avg, y_avg);
    fflush(stdout);
    check_Avg(3);
     return 0;
}
