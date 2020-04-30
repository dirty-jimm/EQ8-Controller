#include "system_calls.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "EQ8-Driver.h"
#include <sys/time.h>

int main()
{
    FILE *csv = fopen("error.csv", "w+");
    int i = 0;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long time_offset = 1000000 * tv.tv_sec + tv.tv_usec;
    system("/bin/stty raw");
    do
    {
        printf("Samples: %i\r", i++);
        fflush(stdout);
        gettimeofday(&tv, NULL);
        unsigned long time_in_micros = 1000000 * tv.tv_sec + tv.tv_usec - time_offset;

        fprintf(csv, "%8lu, %i, %i \n", time_in_micros, get_Analog(1), get_Analog(2));
    } while (!(kbhit() && getchar() == 'c'));
    system("/bin/stty cooked");
    fclose(csv);
    return 1;
}
