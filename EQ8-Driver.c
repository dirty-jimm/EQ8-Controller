/*--------------------------------------------------------------
* EQ8 - Driver
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 11/02/2020
* Last modifiied on: 21/02/2020
* 
* This library contains mid level mount controlling functionality:
*   - Parsing of error codes.
*   - Command retries and logical error handling.
* This should be the highest-level library common to all controllers.
*-------------------------------------------------------------*/

#define VERSION_DRIVER 1.7
#define MAX_INPUT 32
#define STEPS_PER_REV_CHANNEL 0xA9EC00
#include "EQ8-Comms.c"

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
 * */
int convert_Command(char data_in[8], char data_out[8])
{
    unsigned long length = strlen(data_in);
    if (length < 7 && verbose)
    {
        printf("DRIVER_DEBUG: Convert stringlen error, length: %lu\n", length);
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
        printf("Converted:\t%s\n", data_out_temp);
    return 1;
}

/* *
 * Positional data sent from the mount is formatted:    0xefcdab
 * This function reformats for readability to:          0xabcdef
 * Note that all responses from mount are RC terminated, hence 8 Bytes
 * Takes the data array and an output buffer as arguments
 * If output buffer == 0, fuction prints conversion only
 * */
int convert_Response(char data_in[8], char data_out[8])
{
    unsigned long length = strlen(data_in);
    if (length < 7 && verbose)
    {
        printf("DRIVER_DEBUG: Convert stringlen error, length: %lu\n", length);
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
        printf("Converted:\t%s\n", data_out_temp);
    return 1;
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
    int success = flag;
    strcpy(data, (*response).data);
    printf("Response:\t%s\n", data);
    if (flag == -1 && verbose)
    {
        printf("DEBUG: Read Error");
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

/* *
 * Function to send a command to the mount and recieve
 * its response.
 * Returns a pointer to the structure containing flag & data
 * TODO: include retry functionality.
 * */
struct response *send_Command(char command[])
{
    TX(port, command);
    usleep(30000); // this gives the mount time to repsond
    return RX(port);
}

unsigned long angle_to_argument(int channel, int angle)
{
    char curr_Pos_String[8];
    if (channel == 1)
        convert_Response((*send_Command("j1")).data, curr_Pos_String);
    else if (channel == 2)
        convert_Response((*send_Command("j2")).data, curr_Pos_String);
    else
        return -1;
    curr_Pos_String[0] = '0';
    curr_Pos_String[7] = '0';
    unsigned long curr_Pos = strtol(curr_Pos_String, NULL, 16);
    unsigned long target_Pos = (curr_Pos + (angle * (STEPS_PER_REV_CHANNEL / 360))) % 0xA9EC00;
    if (verbose)
    {
        printf("DRIVER_DEBUG: Current position: %06lX\n", curr_Pos);
        printf("DRIVER_DEBUG: Target position: %06lX\n", target_Pos);
    }
    return target_Pos;
}

int go_to(int channel, char target[MAX_INPUT], bool isFormatted)
{
    char command[10];
    command[0] = 'S';
    if (channel == 1)
        command[1] = '1';
    else if (channel == 2)
        command[1] = '2';
    else
        return -1;

    strcat(command, target);
    if (!isFormatted)
        convert_Command(command, command);

    send_Command("K2");
    send_Command(command);
    send_Command("G201");
    send_Command("J2");
    return 1;
}

/* *
 * Function to interpret keyboard commands.
 * Allows for command strings to be defined for more complex
 * or series of commands
 * */
void parse_Command(char input[MAX_INPUT])
{
    char c = '\0';

    if (strcasecmp(input, "help") == 0)
    {
        printf("Driver version %.2f\n", VERSION_DRIVER);
        printf("Comms version %.2f\n\n", VERSION_COMMS);
        printf("Commands:\n");
        printf("position\tContinuously prints position of mount axis.\n");
        printf("manual\t\tAllows user to manually control position of mount.\n");
        printf("go*\t\tMoves mount to a given target position. * specifies axis (1 or 2).\n");
        printf("exit\t\tSevers port connection and quits program.\n\n");
    }

    else if (strcasecmp(input, "position") == 0)
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
            convert_Command((*send_Command(channel)).data, data);
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
                send_Command("K2");
                send_Command("G230");
                send_Command("J2");
                while (!kbhit())
                {
                }
                send_Command("K2");
            }

            else if (c == 'e')
            {
                send_Command("G231");
                send_Command("J2");
                while (!kbhit())
                {
                }
                send_Command("K2");
            }

            else if (c == 'a')
            {
                send_Command("G230");
                send_Command("J2");
                usleep(250000);
                send_Command("K2");
            }

            else if (c == 'd')
            {
                send_Command("G231");
                send_Command("J2");
                usleep(250000);
                send_Command("K2");
            }

            else if (c == 'A')
            {
                send_Command("G230");
                send_Command("J2");
                usleep(500000);
                send_Command("K2");
            }

            else if (c == 'D')
            {
                send_Command("G231");
                send_Command("J2");
                usleep(500000);
                send_Command("K2");
            }

        } while (c != 'c');
        system("/bin/stty cooked");
        send_Command("K1");
        send_Command("K2");
    }

    else if (strcasecmp(input, "go1") == 0)
    {
        char target[MAX_INPUT];
        printf("\nEnter target:\t");
        scanf("%s", target);
        go_to(1, target, false);
    }

    else if (strcasecmp(input, "go2") == 0)
    {
        char target[MAX_INPUT];
        printf("\nEnter target:\t");
        scanf("%s", target);
        go_to(2, target, false);
    }

    else if (strcasecmp(input, "turn") == 0)
    {
        char angle[8];
        while (c != '1' && c != '2')
        {
            printf("\rChannel 1 / 2? ");
            c = getchar();
        }
        int channel = 0;
        if (c == '1')
            channel = 1;
        else if (c == '2')
            channel = 2;

        printf("\nEnter angle:\t");
        scanf("%s", angle);

        long target = angle_to_argument(channel, atoi(angle));
        if (verbose)
            printf("Target: %lX\n", target);

       // char target_C[8];
        //ltoa(target, target_C, 16, 8);
        //Need way to convert long to char[]
        //go_to(channel, target_C, false);
    }

    else if (strcasecmp(input, "exit") == 0 || strcasecmp(input, "quit") == 0)
    {
        shutdown_Controller(port);
        exit(1);
    }

    else
        parse_Response((send_Command(input)));
}

int main(int argc, char **argv)
{
    if (argc > 1 && strcasecmp(argv[1], "verbose") == 0)
    {
        verbose = 1;
    }
    system("clear");
    port = setup_Controller();
    char input[32];
    while (true)
    {
        printf("\nCommand:\t");
        scanf("%s", input);
        parse_Command(input);
    }
}
