/*--------------------------------------------------------------
* EQ8 - SlowFeedback
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 26/05/2020
* Last modifiied on: 09/06/2020
* 
* This library contains high level mount controlling functionality:
*   
*-------------------------------------------------------------*/
#define VERSION_SLOW_FEEDBACK 2.14
#define K_i 0.01
#define OUTER_CLIPPING 1000
#define INNER_CLIPPING 5
#define SAMPLE_FREQUENCY 1

#include <time.h>

int PID_controller()
{
    time_t prev = time(NULL);
    int X_curr, Y_curr;
    static int X_prev, Y_prev;
    double xi = 0.0, yi = 0.0;

    while (true)
    {
        if (time(NULL) - prev >= 1)
        {
            prev = time(NULL);

            X_curr = map(get_Analog(1), -230, 8170, 0, 80) - 80;
            Y_curr = map(get_Analog(2), -230, 3515, 0, 80) - 80;
            printf("Xcurr: %i, Ycurr: %i, ", X_curr, Y_curr);

            xi = xi + K_i * ((X_curr + X_prev) / 2);
            yi = K_i * (yi + ((Y_curr + Y_prev) / 2));
            X_prev = X_curr;
            Y_prev = Y_curr;
            printf("Ki %f\n", xi);
        }
    }
}
