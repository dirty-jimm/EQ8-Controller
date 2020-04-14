/*--------------------------------------------------------------
* EQ8 - Controller
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 12/03/2020
* Last modifiied on: 17/03/2020
* 
* This library contains high level mount controlling functionality:
*   
*-------------------------------------------------------------*/
#define VERSION_CONTROLLER 2.0
#include "EQ8-Driver.c"
#define MAX_INPUT 128

/**
 * Function to return the target position of the mount in order
 * to turn it a given number of degrees
 * NOTE:    Will likely be best to have this return char[], char[] to long is trivial
 *          long to char[] not so
 **/
unsigned long angle_to_argument(int channel, int angle)
{
    char curr_Pos_String[8];
    if (channel == 1)
        convert_Response((*send_Command("j1")).data, curr_Pos_String);
    else if (channel == 2)
        convert_Response((*send_Command("j2")).data, curr_Pos_String);
    else
        return -1;
    curr_Pos_String[0] = '0'; //strips the leading '='
    unsigned long curr_Pos = strtol(curr_Pos_String, NULL, 16);
    unsigned long target_Pos = (curr_Pos + (angle * (STEPS_PER_REV_CHANNEL / 360))) % 0xA9EC00;
    if (verbose)
    {
        printf("DRIVER_DEBUG(angle_to_argument): Current position: %06lX\n", curr_Pos);
        printf("DRIVER_DEBUG(angle_to_argument): Target position: %06lX\n", target_Pos);
    }

    return target_Pos;
}

/**
 * Function to move mount to target position.
 * Channel specifies which axis
 * Target is the target encoder position as a character array
 * isFormatted specifies whether target is formatted for readability (0xabcdef),
 * or in the mounts format (0xefcdab), allowing for either to be used.
 * Returns 1 on success, -1 on failure, including any errors thrown by mount.
 **/
int go_to(int channel, char target[MAX_INPUT], bool isFormatted)
{
    if (channel == 1)
        send_Command("K1");
    else if (channel == 2)
        send_Command("K2");
    else
        return -1;

    char command[10]; //Less than 10 causes problems
    command[0] = 'S';
    if (channel == 1)
        command[1] = '1';
    else if (channel == 2)
        command[1] = '2';
    command[2] = '\0'; //strcat needs null terminator

    strcat(command, target);

    if (!isFormatted)
        convert_Command(command, command);
    if (channel == 1)
    {
        send_Command(command);
        send_Command("G101");
        send_Command("J1");
    }
    else if (channel == 2)
    {
        send_Command(command);
        send_Command("G201");
        send_Command("J2");
    }
    return 1;
}

char *get_Position()
{
    static char data[32];

    char command[3] = "j1";
    char data1[6], data2[6];

    command[1] = '1';
    convert_Response((*send_Command(command)).data, data1);

    command[1] = '2';
    convert_Response((*send_Command(command)).data, data2);
    strcpy(data, "1: ");
    strcat(data, data1);
    strcat(data, ", 2: ");
    strcat(data, data2);
    return data;
}

/* *
 * Function to interpret keyboard commands.
 * Allows for command strings to be defined for more complex,
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
        printf("go*\t\tMoves mount to a given target (encoder) position. * specifies axis (1/2).\n");
        printf("turn\t\tMoves mount a given number of degrees.\n");
        printf("exit\t\tSevers port connection and quits program.\n\n");
    }

    else if (strcasecmp(input, "position") == 0)
    {

        printf("Position Mode\nPress 'c' to exit\n");
        system("/bin/stty raw");
        do
        {
            printf("%s\r", get_Position());
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

    else if (strcasecmp(input, "go") == 0)
    {
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

        char target[6];
        printf("\nEnter target:\t");
        scanf("%s", target);
        if (channel == 1)
            go_to(1, target, false);
        else if (channel == 2)
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
        char target_C[6];
        sprintf(target_C, "%06lX", target);
        if (verbose)
        printf("CONTROLLER_DEBUG(parse_COMMAND: Target(char): %s\n", target_C);
        go_to(channel, target_C, false);
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
        verbose = 1;

    system("clear");
    port = begin_Comms();
    char input[MAX_INPUT];
    while (true)
    {
        printf("Command:");
        scanf("%s", input);
        parse_Command(input);
    }
}
