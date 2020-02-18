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
 * This function reformats for readability to:          0xabcdef
 * Takes the data array and an output buffer as arguments
 * If output buffer == 0, fuction prints conversion only
 * */
void convert(char data_in[9], char data_out[7])
{
    char data_out_temp[7] = {data_in[5],
                             data_in[6],
                             data_in[3],
                             data_in[4],
                             data_in[1],
                             data_in[2]};
    if (data_out != 0)
        strcpy(data_out, data_out_temp);
    else
        printf("Converted:\t%s", data_out_temp);
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
            convert(data, 0);
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
                   // else all responses will be offset by 1 from their respective commands
                   // Should be able to replace this by changing serial to canonical mode
                   // if it causes significant delays later, for now is good
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

        char data[7];
        system("/bin/stty raw");
        do
        {
            convert((*send_Command(port, channel)).data, data);
            printf("\rPosition:\t%s", data);
            fflush(stdout);
        } while (!(kbhit() && getchar() == 'c'));
        system("/bin/stty cooked");
    }

    else if (strcasecmp(input, "manual") == 0)
    {
        printf("Manual Mode: Use 'q' and 'r' to track left and right\nUse 'a' and 'd' to step left and right\nPress 'c' to cancel\n");
        system("/bin/stty raw");
        do
        {
            c = getchar();

            if (c == 'q')
            {
                send_Command(port, "K2");
                send_Command(port, "G230");
                send_Command(port, "J2");
                while (!kbhit())
                {
                }
                send_Command(port, "K2");
            }

            else if (c == 'e')
            {
                send_Command(port, "G231");
                send_Command(port, "J2");
                while (!kbhit())
                {
                }
                send_Command(port, "K2");
            }

            else if (c == 'a')
            {
                send_Command(port, "G230");
                send_Command(port, "J2");
                usleep(250000);
                send_Command(port, "K2");
            }

            else if (c == 'd')
            {
                send_Command(port, "G231");
                send_Command(port, "J2");
                usleep(250000);
                send_Command(port, "K2");
            }

            else if (c == 'A')
            {
                send_Command(port, "G230");
                send_Command(port, "J2");
                usleep(500000);
                send_Command(port, "K2");
            }

            else if (c == 'D')
            {
                send_Command(port, "G231");
                send_Command(port, "J2");
                usleep(500000);
                send_Command(port, "K2");
            }
            
        } while (c != 'c');
        system("/bin/stty cooked");
        send_Command(port, "K1");
        send_Command(port, "K2");
    }

    else if (strcasecmp(input, "exit") == 0 || strcasecmp(input, "quit") == 0)
    {
        shutdown_Controller(port);
        exit(1);
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
