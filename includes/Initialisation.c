/*--------------------------------------------------------------
* EQ8 - Inititilisation
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 28/04/2020
* Last modifiied on: 07/08/2020
*
* This library contains scanning functionailty.
*-------------------------------------------------------------*/
#define VERSION_INITIALISATION 2.14

//Laser properties
#define W0 0.017           //Initial beam waist
#define LAMBDA 0.000001550 // Wavelength of laser
#define SCALE 0.4          // Scale factor

struct point
{
    unsigned long pos_1; //channel 1
    unsigned long pos_2; //and 2 position
    float reading;       //power reading
    int direction_X;     //direction x axis was travelling in
    int direction_Y;     //direction y axis was travelling in
};

/**Function to calculate the appropriate scanning resolution for a given link distance
 * Based on modelling of beam width at that distance.
 **/
double get_Resolution(unsigned long range)
{
    double Zr = (M_PI * pow(W0, 2)) / LAMBDA;
    return SCALE * W0 * sqrt(1 + pow((range / Zr), 2));
}

/**Function to command mount to scan along an axis
 * 
 **/
int scanline2(unsigned long next_X_pos, FILE *csv, int threshold)
{

    go_to(2, lu_to_string(next_X_pos), false);
    while (get_Status(2)) //wait while mount is moving
    {
        float reading = get_Fast_Analog(1, 5);
        printf("Power: %f\n", reading);
        if (reading > threshold)
        {
            unsigned long X = get_Position(1);
            unsigned long Y = get_Position(2);
            printf("%06lX, %06lX,  %f\a\n", X, Y, reading);
        }
    }
    return 0;
}

/**Function to initiate scan
 * range refers to straight line link distance (meters)
 * field refers to desired region to be scanned (degrees)
 **/
int scan(unsigned long range, double field)
{
    time_t start = time(NULL), curr;

    //Open file to save data.
    char filename[64];
    sprintf(filename, "%lu-%0.3f.csv", range, field);
    FILE *csv = fopen(filename, "w+");

    //Configure parameters
    double resolution_m = get_Resolution(range);                          //resolution in meters
    double resolution_a = resolution_m / range;                           //resolution in rads
    int resolution = resolution_a / ((2 * M_PI) / STEPS_PER_REV_CHANNEL); //resolution in encoder steps
    if (resolution < 2)                                                   //mount can run into issues when trying to single step
        resolution = 2;

    double field_rads = (2 * M_PI * field) / 360;
    int max_steps = (field/360)*STEPS_PER_REV_CHANNEL;
    int lines = (field_rads / resolution_a); //number of points in the longest line;

    // Upper and lower limit of x, mount will slew between these two points, stepping in y axis each time.
    const unsigned long start_X = get_Position(2);
    const unsigned long start_Y = get_Position(1);
    unsigned long x_upper = start_X + (max_steps / 2);
    unsigned long x_lower = start_X - (max_steps / 2);
    unsigned long y_upper = start_Y + (max_steps / 2);
    unsigned long next_X_pos = x_upper; // start point
    unsigned long next_Y_pos = y_upper;

    if (verbose)
    {
        printf("\nINITIALIASTION_DEBUG(scan): Link distance: %lu meters\n", range);
        printf("INITIALIASTION_DEBUG(scan): Link field: %f degrees\n", field);
        printf("INITIALIASTION_DEBUG(scan): Link field: %f rads\n", field_rads);
        printf("INITIALIASTION_DEBUG(scan): Scan Resolution: %f rads\n", resolution_a);
        printf("INITIALIASTION_DEBUG(scan): Scan Resolution: %i encoder steps\n", resolution);
        printf("INITIALIASTION_DEBUG(scan): Max Steps: %i scan steps\n", max_steps);

        printf("INITIALIASTION_DEBUG(scan): start x: %lX\n", start_X);
        printf("INITIALIASTION_DEBUG(scan): start y: %lX\n", start_Y);
        printf("INITIALIASTION_DEBUG(scan): x_upper: %lX\n", x_upper);
        printf("INITIALIASTION_DEBUG(scan): x_lower: %lX\n", x_lower);
    }

    if (range <= 0 || field <= 0 || resolution < 1) //Error
        return -1;

    //Move to start position
    go_to(1, lu_to_string(next_Y_pos), false);
    go_to(2, lu_to_string(next_X_pos), false);
    

    int completed = 0; // number of lines that have been scanned
    do
    {

        scanline2(next_X_pos, csv, 3); //scan the next line
        completed++;
       
         if (completed % 2 == 0)
            next_X_pos = x_upper;
        else
            next_X_pos = x_lower;
        next_Y_pos = next_Y_pos - resolution;

        printf("Next: %06lX\n", next_Y_pos);
        go_to(1, lu_to_string(next_Y_pos), false);
        
        curr = time(NULL);
        double percentage = ((double)completed * 100) / (double)lines;
        printf("Progress: %.1f%%, Time remaining: %.0f minutes\n", percentage, (((curr - start) / (percentage / 100)) - (curr - start)) / 60);

    } while (completed < lines);

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
