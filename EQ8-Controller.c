/*--------------------------------------------------------------
* EQ8 - Controller
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 12/03/2020
* Last modifiied on: 30/04/2020
* 
* This library contains high level mount controlling functionality:
*   
*-------------------------------------------------------------*/
#define VERSION_CONTROLLER 2.0
#include "EQ8-Driver.h"
#include "system_calls.h"
#include "EQ8-Initialisation.c"

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
            printf("1: %06luX,2: %06luX\r", get_Position(1), get_Position(2));
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
        else
        {
            printf("Invalid channel number");
            return;
        }

        printf("\rEnter angle:\t");
        scanf("%s", angle);

        long target = angle_to_argument(channel, atof(angle));
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
    else if (strcasecmp(input, "scan") == 0)
    {
        char range[MAX_INPUT];
        printf("\nEnter link range (meters):\t");
        scanf("%s", range);

        unsigned long range_lu = strtol(range, NULL, 10);
        scan(range_lu);
    }
    else if (strcasecmp(input, "status") == 0)
    {
        printf("Status: %i\n", get_Status(2));
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
