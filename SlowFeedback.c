/*--------------------------------------------------------------
* EQ8 - SlowFeedback
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 26/05/2020
* Last modifiied on: 27/05/2020
* 
* This library contains high level mount controlling functionality:
*   
*-------------------------------------------------------------*/
#define VERSION_SLOW_FEEDBACK 2.13
#define K_i 0.2
#define OUTER_CLIPPING 1000
#define INNER_CLIPPING 50

int PID_controller()
{
    time_t start = time(NULL), curr;
    int X_curr, Y_curr, X_prev = 0, Y_prev = 0;
    double xi = 0.0, yi = 0.0;
    //double Ts = 1 / SAMPLE_FREQ;

    while (true)
    {
        X_curr = get_Analog(1) - 2000;
        Y_curr = get_Analog(2) - 2000;
        curr = time(NULL);
        time_t Ts = curr - start;
        xi = K_i * (xi + ((X_curr + X_prev) / 2) * Ts);
        yi = K_i * (yi + ((Y_curr + Y_prev) / 2) * Ts);

        if (xi >= 0 && xi < INNER_CLIPPING)
                xi = 0;

        else if (xi <= 0 && xi > -INNER_CLIPPING)
                xi = 0;

        if (xi > OUTER_CLIPPING)
            xi = OUTER_CLIPPING;

        else if (xi < -OUTER_CLIPPING)
            xi = -OUTER_CLIPPING;

        if (yi > OUTER_CLIPPING)
            yi = OUTER_CLIPPING;

        else if (yi < -OUTER_CLIPPING)
            yi = -OUTER_CLIPPING;

        X_prev = X_curr;
        Y_prev = Y_curr;
        printf("Ki %f, Ts: %lu\n", xi, Ts);
    }
}