/*--------------------------------------------------------------
* EQ8 - Inititilisation
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 28/04/2020
* Last modifiied on: 26/05/2020
*
* This library contains scanning functionailty.
*-------------------------------------------------------------*/
#define VERSION_INITIALISATION 2.13
#include <time.h>

//Laser properties
#define W0 0.017           //Initial beam waist
#define LAMBDA 0.000001550 // Wavelength of laser
#define SCALE 0.4          // Scale factor

struct point
{
    unsigned long pos_1; //channel 1
    unsigned long pos_2; //and 2 position
    int reading;         //power reading
    int direction_X;     //direction x axis was travelling in
    int direction_Y;     //direction y axis was travelling in
};

/**Function to calculate the appropriate scanning resolution for a given link distance
 * Based on modelling of beam width at that distance.
 **/
double get_Resolution(unsigned long range)
{
    double Zr = (M_PI * pow(W0, 2)) / LAMBDA;
    double Wz = SCALE * W0 * sqrt(1 + pow((range / Zr), 2));     // Beam width
    double resolution_a = Wz / range;                            //resolution in rads
    int step_size = resolution_a / ((2 * M_PI) / STEPS_PER_REV); //resolution in encoder steps

    if (step_size < 1)
        step_size = 1;
    if (verbose)
        printf("\nINITIALIASTION_DEBUG(get_Resolution): Wz: %.10f meters\n", Wz);
    return step_size;
}

/**Function to command mount to scan along an axis
 * 
 **/
int scanline(int axis, int resolution, int direction, int steps, FILE *csv, struct point *max_Point, int threshold)
{
    //printf("Axis %i, res %i, dir %i, steps %i\n", axis, resolution, direction, steps);
    int count = 0, reading = 0, reading2 = 0;
    do
    {
        unsigned long curr_Pos = get_Position(axis);
        unsigned long next_Pos = curr_Pos + (direction * resolution);

        if (verbose)
        {
            printf("Curr: %06lX\n", curr_Pos);
            printf("Next: %06lX\n", next_Pos);
        }

        //NEXTPOS:
        go_to(axis, lu_to_string(next_Pos), false);
        while (get_Status(axis)) //wait while mount is moving
        {
        }

        //if (get_Position(axis) != next_Pos)
        //  goto NEXTPOS;

        reading = get_Analog(1, 10);
        reading2 = get_Analog(2, 10);
        unsigned long X = get_Position(1);
        unsigned long Y = get_Position(2);

        fprintf(csv, "%06lX,%06lX,%i,%i\n", X, Y, reading, reading2);
        printf("%06lX, %06lX,  %i", X, Y, reading);
        if (reading > max_Point->reading)
        {
            max_Point->reading = reading;
            max_Point->pos_1 = X;
            max_Point->pos_2 = Y;
            max_Point->direction_X = direction;
            max_Point->direction_Y = direction;
            if (axis == 2)
                max_Point->direction_X = direction * (-1);

            printf(" - MAXIMUM");
        }
        printf("\n");

        if (reading > threshold)
            return 1;

        count++;

    } while (count <= steps);

    return 0;
}

/**Function to initiate scan
 * range refers to straight line link distance (meters)
 * field refers to desired region to be scanned (degrees)
 **/
int scan(unsigned long range, double field)
{
    time_t start = time(NULL), curr;
    int step_size = get_Resolution(range);
    double field_rads = (2 * M_PI * field) / 360;
    int max_steps = (field_rads / step_size);            //number of points in the longest line
    int total_steps = max_steps * max_steps + max_steps; //total number of points to be measured
    char filename[64];
    sprintf(filename, "Data/%lu-%0.3f.csv", range, field);
    FILE *control = fopen("control.csv", "r+");
    if (control == NULL)
    {
        printf("Control file does not exist, generating...\n");
        control = fopen("control.csv", "w+");
    }
    else
    {
        printf("\nContinue previous scan?");
        char c = '\0';
    READINPUT:
        c = getchar();
        if (c == 'y' || c == 'Y')
        {
        }

        else if (c == 'n' || c == 'N')
        {
        }
        else
            goto READINPUT;
    }

    fprintf(control, "\nRange: %lX, Field: %0.3f", range, field);
    FILE *csv = fopen(filename, "w+");

    if (verbose)
    {
        printf("\nINITIALIASTION_DEBUG(scan): Distance: %lu meters\n", range);
        printf("INITIALIASTION_DEBUG(scan): Scan field: %.3f degrees\n", field);
        printf("INITIALIASTION_DEBUG(scan): Scan field: %.3f rads\n", field_rads);
        printf("INITIALIASTION_DEBUG(scan): Scan Resolution: %f rads\n", (2 * M_PI * step_size) / (STEPS_PER_REV));
        printf("INITIALIASTION_DEBUG(scan): Step size: %i encoder ticks\n", step_size);
    }
    if (range <= 0 || field <= 0 || step_size < 1) //Error
        return -1;

    int axis = 1;      //Axis motor to turn
    int direction = 1; //The direction this line going to be scanned
    int steps = 1;     //How man steps are in this line, first line has 1
    int found = 0;     //Has a reading above a minimum threshold been found?
    int completed = 0;

    int reading = get_Analog(1, 10); //read the intensity of returned power.
    int reading2 = get_Analog(2, 10);
    unsigned long X = get_Position(1);
    unsigned long Y = get_Position(2);

    struct point max_Point = {X, Y, reading, 0, 0};
    fprintf(csv, "%06lX,%06lX,%i,%i\n", X, Y, reading, reading2);

    do
    {
        if (axis == 0) // if 0:
        {
            axis = 2;                   //make it 2
            steps++;                    //increase number of steps on the next two lines
            direction = direction * -1; //reverse scan direction
        }

        found = scanline(axis, step_size, direction, steps, csv, &max_Point, 10000); //scan the next line

        completed = completed + steps;
        axis = (axis + 1) % 2; // change axis, can either be 1 or0
        curr = time(NULL);
        double percentage = ((double)completed * 100) / (double)total_steps;
        printf("Progress: %.1f%%, Time remaining: %.0f minutes\n", percentage, (((curr - start) / (percentage / 100)) - (curr - start)) / 60);

        if (found == 1)
            break;

    } while (steps <= max_steps);
    fprintf(control, "%i, %i, %i, %i, %i, %i", max_steps, total_steps, axis, direction, steps, completed);

    //go_to the coordinates corresponding to max intensity
    printf("Going to: %lX, %lX\n", max_Point.pos_1, max_Point.pos_2);
    go_to(1, lu_to_string(max_Point.pos_1), false);
    go_to(2, lu_to_string(max_Point.pos_2), false);

    fclose(csv);
    return 1;
}

int get_Scan_Parameters()
{
    char range[MAX_INPUT];
    char field[MAX_INPUT];
    printf("\nEnter link range:\t");
    scanf("%s", range);
    printf("%sm", range);
    printf("\nEnter link field:\t");
    scanf("%s", field);
    printf("%s\u00B0", field);

    unsigned long range_lu = strtol(range, NULL, 10);
    double field_d = strtod(field, NULL);
    return scan(range_lu, field_d);
}
