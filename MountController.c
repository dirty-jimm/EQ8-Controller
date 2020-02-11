/*--------------------------------------------------------------
* Mount Controller
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 11/02/2020
* Last modifiied on: 11/02/2020
*
* This program contains high level mount controlling functionality:
*   - Parsing of text commands.
    - Parsing of error codes
*   - Command retries and logical error handling.
*   - 
*-------------------------------------------------------------*/

#define VERSION_CONTROLLER 1.1
#include "EQ8-Driver.c"

int parse_Response(struct response *response)
{
    int flag = (*response).flag;
    char data[9];
    strcpy(data,(*response).data);
    if (flag == -1)
    {
        printf("DEBUG: Read Error");
    }

    else if (flag == 1)
    {
        if (data[0] == '!')
        {
            printf("Mount Error: ");

            if (data[1] == '0')
                printf("Unknown Command\n");
            if (data[1] == '1')
                printf("Command Length\n");
            if (data[1] == '2')
                printf("Motor Not Stopped\n");
            if (data[1] == '3')
                printf("Invalid Characer\n");
            if (data[1] == '4')
                printf("Not Initialised\n");
            if (data[1] == '5')
                printf("Driver Sleeping\n");
        }
    }
    return flag;
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
        printf("\nCommand:");
        scanf("%s", input);
        send_Command(fd, input);
        usleep(30000); // this gives the mount time to repsond
        // else all responsed will be offset by 1 from their respective commands
        // Should be able to replace this by changing serial to canonical mode
        parse_Response(read_Response(fd));
    }

    close(fd); /* Close the serial port */
}
