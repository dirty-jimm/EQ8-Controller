/*--------------------------------------------------------------
* EQ8 - Inititilisation
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 28/04/2020
* Last modifiied on: 11/05/2020po
*-------------------------------------------------------------*/
#define VERSION_Initialisation 2.0
#include <time.h>

//Laser properties
#define W0 0.017           //Initial beam waist
#define LAMBDA 0.000001550 // Wavelength of laser
#define SCALE 0.2          // Scale factor

struct point
{
    unsigned long pos_1, pos_2; //channel 1 and 2 position
    int reading;                //power reading
    int direction;              //direction axis was travelling in
    int axis;                   //axis that was being traversed
    int alt_direction;          //direction that the previous axis was travelling in
};

double get_Resolution(unsigned long range)
{
    double Zr = (M_PI * pow(W0, 2)) / LAMBDA;
    return SCALE * W0 * sqrt(1 + pow((range / Zr), 2));
}

int scanline(int axis, int resolution, int direction, int steps, FILE *csv, int *max, unsigned long *Xp, unsigned long *Yp)
{
    //printf("Axis %i, res %i, dir %i, steps %i\n", axis, resolution, direction, steps);
    int count = 0, reading = 0, threshold = 0;
    do
    {
        unsigned long curr_Pos = get_Position(axis);
        unsigned long next_Pos = curr_Pos + (direction * resolution);

        if (verbose)
        {
            printf("Curr: %06lX\n", curr_Pos);
            printf("Next: %06lX\n", next_Pos);
        }

        go_to(axis, lu_to_string(next_Pos), false);
        while (get_Status(axis))
        {
            //wait while mount is moving
        }

        reading = get_Analog(1);

        if (reading > *max)
        {
            *max = reading;
            *Xp = get_Position(1);
            *Yp = get_Position(2);
            printf("\nMAXIMUM - %06lX, %06lX,  %i\n", get_Position(1), get_Position(2), reading);
        }

        fprintf(csv, "%06lX,%06lX, %i\n", get_Position(1), get_Position(2), reading);
        printf("%06lX, %06lX,  %i\n", get_Position(1), get_Position(2), reading);

        if (reading > 10000)
        {
            threshold = 1;
            break;
        }

        count++;
    } while (count <= steps);
    return threshold;
}

int scan(unsigned long range, double field)
{
    double resolution_m = get_Resolution(range);                          //resolution in meters
    double resolution_a = resolution_m / range;                           //resolution in rads
    int resolution = resolution_a / ((2 * M_PI) / STEPS_PER_REV_CHANNEL); //resolution in encoder steps
    double field_rads = (2 * M_PI * field) / 360;
    int max_steps = (field_rads / resolution_a);         //number of points in the longest line
    int total_steps = max_steps * max_steps + max_steps; //total number of points to be measured

    //if (verbose)
    printf("\nINITIALIASTION_DEBUG(scan) - Link distance: %lu meters\n", range);
    printf("INITIALIASTION_DEBUG(scan) - Link field: %f degrees\n", field);
    printf("INITIALIASTION_DEBUG(scan) - Link field: %f rads\n", field_rads);
    printf("INITIALIASTION_DEBUG(scan) - Scan Resolution: %f rads\n", resolution_a);
    printf("INITIALIASTION_DEBUG(scan): Scan Resolution: %i encoder steps\n", resolution);
    printf("INITIALIASTION_DEBUG(scan): Max Steps: %i scan steps\n", max_steps);
    printf("INITIALIASTION_DEBUG(scan): Total Steps: %i scan steps\n", total_steps);

    int axis = 2;                     //Axis motor to turn
    int direction = 1;                //The direction this line going to be scanned
    int steps = 1;                    //How man steps are in this line
    int max;                          //The highest link reading seen so far
    int found = 0;                    //Has a reading above a minimum threshold been found?
    unsigned long X, Y;               //The coordinates of the highest reading
    int *maxp = &max;                 //Pointer to the max reading
    unsigned long *Xp = &X, *Yp = &Y; //and to its coordinates
    int completed = 0;

    FILE *csv = fopen("scan.csv", "w+");

    int reading = get_Analog(1); //read the intensity

    fprintf(csv, "%06lX,%06lX,%i\n", get_Position(1), get_Position(2), reading);

    *maxp = reading;       //set first reading as max by default
    *Xp = get_Position(1); //^^
    *Yp = get_Position(2); //^^
    time_t start = time(NULL), curr;
    do
    {
        found = scanline(axis, resolution, direction, steps, csv, maxp, Xp, Yp); //scan the next line
        completed = completed + steps;
        axis = (axis + 1) % 2; // change axis, can either be 1 or 0
        if (axis == 0)         // if 0:
        {
            axis = 2;                   //make it 2
            steps++;                    //increase number of steps on the next two lines
            direction = direction * -1; //reverse scan direction
        }

        curr = time(NULL);
        double percentage = ((double)completed * 100) / (double)total_steps;
        time_t diff = curr - start;
        printf("Progress: %.1f%%, Time remaining: %.0f minutes\t\t\r", percentage, ((diff / (percentage / 100)) - diff) / 60);
        fflush(stdout);

        if (found == 1)
            break;

    } while (steps <= max_steps);

    //go_to the coordinates corresponding to max intensity
    printf("Going to: %lX, %lX\n", *Xp, *Yp);
    go_to(1, lu_to_string(*Xp), false);
    go_to(2, lu_to_string(*Yp), false);

    //recursive check
    fclose(csv);
    return 1;
}
