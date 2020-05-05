/*--------------------------------------------------------------
* EQ8 - Inititilisation
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 28/04/2020
* Last modifiied on: 01/05/2020
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

int scanline(int axis, int resolution, int direction, int steps, FILE *csv, int *max, unsigned long *Xp, unsigned long *Yp)
{
    printf("Axis %i, res %i, dir %i, steps %i\n", axis, resolution, direction, steps);
    int count = 0, reading;
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
        }
        fprintf(csv, "%06luX, %06luX,  %i\n", get_Position(1), get_Position(2), reading);
        count++;
    } while (count <= steps);
    return 0;
}

int scan(unsigned long range)
{
    double resolution_m = get_Resolution(range);  //resolution in meters
    double resolution_a = resolution_m / range;   //resolution in rads
    int resolution = resolution_a / RADS_PER_REV; //resolution in encoder steps

    //if (verbose)
    printf("\nINITIALIASTION_DEBUG(scan) - Link distance: %lu meters\n", range);
    printf("INITIALIASTION_DEBUG(scan) - Scan Resolution: %f rads\n", resolution_m);
    printf("INITIALIASTION_DEBUG(scan) - Scan Resolution: %i steps\n", resolution);

    int axis = 2;                     //Axis motor to turn
    int finished = 0;                 //Has the scan finished?
    int direction = 1;                //The direction this line going to be scanned
    int steps = 1;                    //How man steps are in this line
    int max;                          //The highest link reading seen so far
    unsigned long X, Y;               //The coordinates of the highest reading
    int *maxp = &max;                 //Pointer to the max reading
    unsigned long *Xp = &X, *Yp = &Y; //and to its coordinates

    FILE *csv = fopen("scan.csv", "w+");
    int reading = get_Analog(1); //read the intensity
    fprintf(csv, "%06luX, %06luX,  %i\n", get_Position(1), get_Position(2), reading);
    *maxp = reading;       //set first reading as max by default
    *Xp = get_Position(1); //^^
    *Yp = get_Position(2); //^^

    do
    {
        finished = scanline(2, resolution, direction, steps, csv, maxp, Xp, Yp); //scan the next line
        axis = (axis + 1) % 2;                                                   // change axis, can either be 1 or 0
        if (axis == 0)                                                           // if 0:
        {
            axis = 2;                   //make it 2
            steps++;                    //increase number of steps on the next two lines
            direction = direction * -1; //reverse scan direction
        }
    } while (finished == 0);

    //Finish check

    //go_to the coordinates corresponding to max intensity
    go_to(1, lu_to_string(*Xp), false);
    go_to(2, lu_to_string(*Yp), false);

    //recursive check
    fclose(csv);
    return 1;
}
