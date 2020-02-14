/*--------------------------------------------------------------
* EQ8 - Driver
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 11/02/2020
* Last modifiied on: 13/02/2020
* 
* This library contains mid level mount controlling functionality:
*   - Parsing of error codes.
*   - Command retries and logical error handling.
* This should be the highest-level library common to all controllers.
*-------------------------------------------------------------*/

#define VERSION_DRIVER 1.4
#define MAX_INPUT 32
#include "EQ8-Comms.c"


int kbhit()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

/* *
 * Positional data sent from the mount is formatted:    0xefcdab
 * This function reformats for readability to:          0xabcdef
 * */
void convert(char data_in[9])
{
    char data_out[7] = {data_in[5],
                        data_in[6],
                        data_in[3],
                        data_in[4],
                        data_in[1],
                        data_in[2]};
    printf("Converted:\t%s", data_out);
}

/* *
 * Function to controller, call port setup.
 * Returns port ID on success, -1 on failure.
 * */
int setup_Controller(void)
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
 * Determines if response is error (and interprets),
 * or success and presents data.
 * Prints interpretation.
 * Returns flag.
 * */
int parse_Response(struct response *response)
{
    int flag = (*response).flag;
    char data[9];
    strcpy(data, (*response).data);
    printf("Response:\t%s\n", data);
    if (flag == -1 && verbose)
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
        else if (flag && strlen(data) == 8)
            convert(data);
    }
    return flag;
}

/* *
 * Function to send a command to the mount and recieve
 * its response.
 * Returns a pointer to the structure containing flag & data
 * TODO: include retry functionality.
 * Returns port ID on success, -1 on failure.
 * */
struct response *send_Command(int port, char command[])
{
    TX(port, command);
    usleep(30000); // this gives the mount time to repsond
                   // else all responsed will be offset by 1 from their respective commands
                   // Should be able to replace this by changing serial to canonical mode
    return RX(port);
}

/* *
 * Function to interpret keyboard commands.
 * Allows for command strings to be defined for more complex
 * or series of commands
 * */
void parse_Command(int port, char input[MAX_INPUT])
{
    char c = '\0';
    if (strcasecmp(input, "position") == 0)
    {
        printf("Position Mode: \nPress 'c' to cancel\n");
        while (c != '1' && c != '2')
        {
            printf("\rChannel 1 / 2? ");
            c = getchar();
        }
        char channel[3];
        channel[0] = 'j';
        if (c == '1')
            channel[1] = '1';
        else if (c == '2')
            channel[1] = '2';

        system("/bin/stty raw");
        do
        {
            printf("\rPosition:\t%s", (*send_Command(port, channel)).data);
            fflush(stdout);
        }  while (!(kbhit() && getchar() == 'c'));
        system("/bin/stty cooked");
    }

    else if (strcasecmp(input, "manual") == 0)
    {
        printf("Manual Mode: Use 'l' and 'r' to move\nPress 'c' to cancel\n");
        system("/bin/stty raw");

        char c;
        do
        {
            c = getchar();

            if (c == 'l')
            {
                TX(port, "G230");
                usleep(20000);
                TX(port, "J2");
                while (c == 'l')
                {
                    c = getchar();
                }
                TX(port, "K2");
            }

            else if (c == 'r')
            {
                TX(port, "G231");
                usleep(30000);
                TX(port, "J2");
                while (c == 'r')
                {
                    c = getchar();
                }

                TX(port, "K2");
                usleep(30000);
            }
        } while (c != 'c');
        system("/bin/stty cooked");
    }
    else
        parse_Response((send_Command(port, input)));
}

int main(void)
{
    system("clear");
    char input[MAX_INPUT];
    int fd = setup_Controller();
    while (true)
    {
        printf("\nCommand:\t");
        scanf("%s", input);
        parse_Command(fd, input);
    }
}
