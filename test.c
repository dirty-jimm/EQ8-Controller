#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void)
{
    system("/bin/stty raw");

    while (1)
    {
        printf("%i\n", getchar());
    }
    system("/bin/stty cooked");
}
