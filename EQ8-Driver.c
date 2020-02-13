/*--------------------------------------------------------------
* Mount Controller
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 11/02/2020
* Last modifiied on: 13/02/2020
*
* This program contains high level mount controlling functionality:
*   - Parsing of text commands.
    - Parsing of error codes
*   - Command retries and logical error handling.
*   - 
*-------------------------------------------------------------*/

#define VERSION_CONTROLLER 1.4
#include "EQ8-Comms.c"
void convert(char data[9])
{
    printf("%c%c%c%c%c%c\n", data[5], data[6], data[3], data[4], data[1], data[2]);
}

int parse_Response(struct response *response)
{
    int flag = (*response).flag;
    char data[9];
    strcpy(data, (*response).data);
    printf("Response:\t%s\n", data);
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
        convert(data);
    }
    return flag;
}

int send_Command(int port, char command[])
{
    TX(port, command);
    usleep(30000); // this gives the mount time to repsond
                   // else all responsed will be offset by 1 from their respective commands
                   // Should be able to replace this by changing serial to canonical mode
    return parse_Response(RX(port));
}



int setup_Controller(void)
{
    int fd = setup_Port();
    printf("\n--- EQ8 Pro Mount Controller ---\n");
    if (verbose)
        printf("Verbose Mode: on\n");
    if (verbose)
        printf("Version: %.2f\n", VERSION_CONTROLLER);
    printf("----------------------------\n");
    return fd;
}

int shutdown_Controller(int port)
{
    return close(port); /* Close the serial port */
}
