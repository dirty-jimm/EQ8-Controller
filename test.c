#include "system_calls.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "EQ8-Driver.h"
#include <sys/time.h>
int main()
{
    while(1)
    {
        printf("%i, %i \r", get_Analog(1), get_Analog(2));
        //printf("%f, %f \r", map(get_Analog(1),-200,3000,0,10), map(get_Analog(2),-200,3000,0,10));
        fflush(stdout);
    }
    return 1;
}
