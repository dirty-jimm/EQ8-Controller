/*--------------------------------------------------------------
* EQ8 - SlowFeedback
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 26/05/2020
* Last modifiied on: 07/08/2020
* Version  2.3
*-------------------------------------------------------------*/

#define VERSION_SLOW_FEEDBACK 2.14
#define SAMPLES 10000
int x_inner = 10;
int y_inner = 10;
int x_outer = 15;
int y_outer = 15;
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
        if (channel==1)       
        {
            if(error>0)
                curr_pos--;
            else 
                curr_pos++;
        }       
        
        else
         {
             if(error>0)
                curr_pos--;
            else 
                curr_pos++;
         }

        return go_to(channel, lu_to_string(curr_pos), false);
        }
    return 1;
}

int check_Avg(int channel)
{
    int r=0;
    if (channel == 2 || channel == 3)
    {
        if (abs(x_avg) < x_inner)
            r= 0;
        else if (abs(x_avg) < x_outer)
          { 
                actuate(2, x_avg);
                r= 1;
          }
       else  r= 2;
    }

    if (channel == 1 || channel == 3)
    {
        if (abs(y_avg) < y_inner)
            r= 0;
        else if (abs(y_avg) < y_outer)
            {
                actuate(1, y_avg);
                r= 1;
            }
        else r= 2;
    }
    return r;
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
    if(verbose)
      {  
        //  printf("Xcurr: %06.3f, Ycurr: %06.3f\n", error_X, error_Y);
        printf("X: %06.3f, Y: %06.3f\n", x_avg, y_avg);
}
    else 
    printf("X: %06.3f, Y: %06.3f\r", x_avg, y_avg);
    //printf("%06.3f, %06.3f\n", x_avg, y_avg);
    fflush(stdout);
    if(check_Avg(3))
       { 
        memset(Xsamples,0, sizeof(Xsamples));
        memset(Ysamples,0, sizeof(Ysamples));
       }
    return 0;
}
