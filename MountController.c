/*--------------------------------------------------------------
* Mount Controller
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 07/02/2020
* Last modifiied on: 07/02/2020
*
* This program contains high level mount controlling functionality:
*   - Parsing of text commands.
    - Parsing of error codes
*   - Command retries and logical error handling.
*   - 
*-------------------------------------------------------------*/

#define VERSION_CONTROLLER 1.0
#include "EQ8-Driver.c"

int parse_Response(int response)
{
    if (response == 0)
    {
        printf("Acknowledged\n");
    }
    else if (response == 1)
    {
        printf("Error:Unknown Command\n");
    }

    else if (response == 2)
    {
        printf("Error:Command Length\n");
    }

    else if (response == 3)
    {
        printf("Error: Motor Not Stopped\n");
    }

    else if (response == 4)
    {
        printf("Error: Invalid Characer\n");
    }

    else if (response == 5)
    {
        printf("Error: Not Initialised\n");
    }

    else if (response == 6)
    {
        printf("Error: Driver Sleeping\n");
    }

    else if (response == -1)
    {
        printf("Read Error or no response\n");
    }

    return response;
}

int main(void)
{
    int fd = setup_Port();
    printf("\n--- EQ8 Pro Mount Controller ---\n");
    if (verbose)
        printf("Verbose Mode: on\n");
    if (verbose)
        printf("Version: %.2f\n", VERSION_CONTROLLER);
    printf("----------------------------\n");

    char input[32];
    //Test loop to allow keyboard commands, to be removed
    while (1)
    {
        printf("Command:");
        scanf("%s", input);
        send_Command(fd, input);
        usleep(30000); // this gives the mount time to repsond
        // else all responsed will be offset by 1 from their respective commands
        // Should be able to replace this by changing serial to canonical mode
        parse_Response(read_Response(fd));
    }

    close(fd); /* Close the serial port */
}
