/*--------------------------------------------------------------
* EQ8 - Inititilisation
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 12/03/2020
* Last modifiied on: 17/03/2020
*  
* This library contains high level mount controlling functionality:
*   
*-------------------------------------------------------------*/
#define VERSION_Initialisation 2.0

//Laser properties
#define W0 0.017
#define LAMBDA 0.000001550
#define SCALE 0.2
#define RADS_PER_REV 0.00000056422

double get_Resolution(unsigned long range)
{
    double Zr = (M_PI * pow(W0, 2)) / LAMBDA;
    return W0 * sqrt(1 + pow((range / Zr), 2));
}

int scanline(int axis, int resolution, int direction, int steps, FILE *csv)
{
    printf("Axis %i, res %i, dir %i, steps %i\n", axis, resolution, direction, steps);
    int count = 0;
    do
    {
        unsigned long curr_Pos = get_Position(axis);
        if (verbose)
            printf("Curr: %06lX\n", curr_Pos); //remove

        unsigned long next_Pos = curr_Pos + (direction * resolution);
        if (verbose)
            printf("Next: %06lX\n", next_Pos); //remove

        char next_Pos_String[6];
        sprintf(next_Pos_String, "%06lX", next_Pos);

        go_to(axis, next_Pos_String, false);
        while (get_Status(axis)) //&& voltage reading
        {
            //wait while mount is moving
        }
        fprintf(csv, "%06luX, %06luX,  %i\n", get_Position(1), get_Position(2), get_Analog(1));
        count++;
    } while (count <= steps);
    return 0;
}

int scan(unsigned long range)
{
    double resolution_m = get_Resolution(range);
    double resolution_a = resolution_m / range;
    int resolution = resolution_a / RADS_PER_REV;
    //if (verbose)
    printf("\nINITIALIASTION_DEBUG(scan) - Link distance: %lu meters\n", range);
    printf("INITIALIASTION_DEBUG(scan) - Scan Resolution: %f rads\n", resolution_m);
    printf("INITIALIASTION_DEBUG(scan) - Scan Resolution: %i steps\n", resolution);

    int axis = 2;
    int finished = 0;
    int direction = 1;
    int steps = 1;

    FILE *csv = fopen("scan.csv", "w+");
    fprintf(csv, "%06luX, %06luX,  %i\n", get_Position(1), get_Position(2), get_Analog(1));
    do
    {
        finished = scanline(2, resolution, direction, steps, csv);
        axis = (axis+1) % 2; // Axis can either be 1 or 2
        if (axis == 0)       // if 2nd axis,
        {
            axis = 2;                   //set axis to 2
            steps++;                    // increase number of steps on the next two lines
            direction = direction * -1; // reverse scan direction
        }
    } while (finished == 0);

    fclose(csv);
    return 1;
}
