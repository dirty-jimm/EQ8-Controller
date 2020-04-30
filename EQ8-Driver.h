/*--------------------------------------------------------------
* EQ8 - Driver
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 11/02/2020
* Last modifiied on: 11/03/2020
* 
* This library contains low level functionality:
*   - Command formatting.
*   - Parsing of error codes.
*   - Command retries and logical error handling.
* This should be the highest-level library common to all controllers.
*-------------------------------------------------------------*/

#define VERSION_DRIVER 2.0
#define STEPS_PER_REV_CHANNEL 0xA9EC00
#include "EQ8-Comms.h"

int port;
/* *
* Determines if a key has been pressed, non blocking
* Not perfect and should be replaced but works for now
* */
int kbhit()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0; 
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

/* *
 * Positional data sent from the mount is formatted:    0xefcdab
 * This function reformats so commands can be written:  0xabcdef
 * and sent correctly
 * data_in is the unformatted command and must include the 
 * character command flag and channel eg. if setting target position
 * data_in  = "S1abcdef"
 * data_out = "S1efcdab" 
 * */
int convert_Command(char data_in[10], char data_out[8])
{
    if (verbose)
    printf("DRIVER_DEBUG(convert_Command):data_in\t%s\n", data_in);
    
    size_t length = strlen(data_in);
    if (length < 7 && verbose)
    {
        printf("DRIVER_DEBUG(convert_Command): Convert stringlen error, length: %zu\n", length);
        return -1;
    }

    char data_out_temp[8] = {data_in[0],
                             data_in[1],
                             data_in[6],
                             data_in[7],
                             data_in[4],
                             data_in[5],
                             data_in[2],
                             data_in[3]};
    strcpy(data_out, data_out_temp);
    if (verbose)
    {
        
        printf("DRIVER_DEBUG(convert_Command):data_out\t%s\n", data_out);
    }
    return 1;
}

/* *
 * Positional data sent from the mount is formatted:    0xefcdab
 * This function reformats for readability to:          0xabcdef
 * Note that all responses from mount are RC terminated, hence 8 Bytes
 * Takes the data array and an output buffer as arguments
 * If output buffer == 0, fuction prints conversion only
 * To avoid formatting issues, the trailing RC from the mount is
 * replaced with '\0'
 * data_in = "=efcdab\r"
 * data_out = "=abcdef\0"
 * */
int convert_Response(char data_in[8], char data_out[8])
{
    unsigned long length = strlen(data_in);
    if (length < 7 && verbose)
    {
        printf("DRIVER_DEBUG(convert_Response): Convert stringlen error, length: %lu\n", length);
        return -1;
    }

    char data_out_temp[8] = {data_in[0],
                             data_in[5],
                             data_in[6],
                             data_in[3],
                             data_in[4],
                             data_in[1],
                             data_in[2],
                             '\0'};
    if (data_out != 0)
        strcpy(data_out, data_out_temp);
    else if (verbose)
        printf("DRIVER_DEBUG(convert_Response)Converted: %s\n", data_out_temp);
    return 1;
}

/* *
 * Function to setup controller, call port setup.
 * Returns port ID on success, -1 on failure.
 * */
int begin_Comms(void)
{
    int fd = setup_Port();

    if (verbose)
    {
        printf("\nEQ8 Pro Mount Driver:\n");
        printf("Version: %.2f\n\n", VERSION_DRIVER);
    }
    return fd;
}

/* *
 * Function to shutdown port connection.
 * Returns port ID on success, -1 on failure.
 * */
int shutdown_Controller(int port)
{
    return close(port); /* Close the serial port */
}

/* *
 * Function to interpret response from mount
 * Determines if response is error (and reports error type),
 * or success and presents data.
 * Prints interpretation.
 * Returns flag.
 * */
int parse_Response(struct response *response)
{
    int flag = (*response).flag;
    char data[9];
    int success = flag;
    strcpy(data, (*response).data);
    printf("Response: %s\n", data);
    if (flag == -1 && verbose)
    {
        printf("DRIVER_DEBUG(parse_Response): Read Error");
        success = -1;
    }

    else if (flag == 1)
    {
        success = 1;
        if (data[0] == '!')
        {
            printf("Mount Error: ");
            success = 0;
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
        else if (strlen(data) == 8)
            convert_Response(data, 0);
    }
    return success;
}

/**
 * Function to send a command to the mount and recieve
 * its response. Retries TX upon mount error 10 times..E
 * Returns a pointer to the structure containing flag & data
 * */
struct response *send_Command(char command[])
{
    int retries = 0;
    struct response *resp;
    char writebuffer[strlen(command) + 2]; // Create buffer with length of the command +2 for the leading ":" and trailing RC
    strcpy(writebuffer, ":");
    strcat(writebuffer, command);
    strcat(writebuffer, "\r");

SEND:
    TX(port, writebuffer);
    usleep(30000); // this gives the mount time to repsond
    resp = RX(port);
    if ((*resp).data[0] == '!' && (*resp).data[1] != '0' && retries < 10)
    {
        usleep(10000);
        retries++;
        goto SEND; // retry command
    }
    return resp;
}


