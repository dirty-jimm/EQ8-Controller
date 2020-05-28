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
#define VERSION_CONTROLLER 2.13

#include "Driver.c"
#include "system_calls.c"
#include "Initialisation.c"
#include "SlowFeedback.c"

/* *
 * Function to interpret keyboard commands.
 * Allows for command strings to be defined for more complex,
 * or series of commands
 * */
void help(int option)
{
    if (option == 1)
    {
        printf("Driver version %.2f\n", VERSION_DRIVER);
        printf("Comms version %.2f\n\n", VERSION_COMMS);
        printf("Commands:\n");
        printf("position\tContinuously prints position of mount axes.\n");
        printf("manual\t\tAllows user to manually control position of mount.\n");
        printf("go\t\tMoves mount to a given target (encoder) position. * specifies axis (1/2).\n");
        printf("turn\t\tMoves mount a given number of degrees.\n");
        printf("scan\t\tStarts initialisation scan.\n");
        printf("exit\t\tSevers port connection and quits program.\n\n");
        printf("\nAll other inputs are interpreted as commands and are sent to the mount.\n\n");
    }
}

void parse_Command(char input[MAX_INPUT])
{
    char c = '\0';

    if (strcasecmp(input, "help") == 0)
    {
        help(1);
    }
    else if (strcasecmp(input, "position") == 0)
    {
        int temp_verbose = verbose;
        verbose = 0;
        printf("Position Mode\nPress 'c' to exit\n");
        system("/bin/stty raw");
        do
        {
            unsigned long X = get_Position(1);
            unsigned long Y = get_Position(2);
            if (X != -1 && Y != -1)
            {
                printf("\r1: %06lX,2: %06lX\t\t\t", X, Y);
                fflush(stdout);
            }
            else break;

        } while (!(kbhit() && getchar() == 'c'));
        system("/bin/stty cooked");
        printf("\n");
        verbose = temp_verbose;
    }
    else if (strcasecmp(input, "manual") == 0)
    {
        printf("Manual Mode: Press 'c' to cancel\n");
        printf("\nUse 'q' and 'r' to slew left and right");
        printf("\nUse 'a' and 'd' to small step left and right");
        printf("\nUse 'A' and 'A' to large step left and right");
        printf("\nUse 'w' and 'd' to small step up and down");
        printf("\nUse 'W' and 'D' to large step up and down");
        printf("\n");
        system("/bin/stty raw");
        do
        {
            c = getchar();
            unsigned long X = get_Position(1);
            unsigned long Y = get_Position(2);
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
                send_Command("K2");
                send_Command("G231");
                send_Command("J2");
                while (!kbhit())
                {
                }
                send_Command("K2");
            }
            if (c == 'z')
            {
                send_Command("K1");
                send_Command("G130");
                send_Command("J1");
                while (!kbhit())
                {
                }
                send_Command("K1");
            }

            else if (c == 'x')
            {
                send_Command("K1");
                send_Command("G131");
                send_Command("J1");
                while (!kbhit())
                {
                }
                send_Command("K1");
            }

            else if (c == 'a')
            {
                unsigned long next_Pos = Y + 100;
                go_to(2, lu_to_string(next_Pos), false);
            }

            else if (c == 'd')
            {
                unsigned long next_Pos = Y - 100;
                go_to(2, lu_to_string(next_Pos), false);
            }

            else if (c == 'A')
            {
                unsigned long next_Pos = Y + 10000;
                go_to(2, lu_to_string(next_Pos), false);
            }

            else if (c == 'D')
            {
                unsigned long next_Pos = Y - 10000;
                go_to(2, lu_to_string(next_Pos), false);
            }
            else if (c == 'w')
            {
                unsigned long next_Pos = X + 100;
                go_to(1, lu_to_string(next_Pos), false);
            }

            else if (c == 's')
            {
                unsigned long next_Pos = X - 100;
                go_to(1, lu_to_string(next_Pos), false);
            }

            else if (c == 'W')
            {
                unsigned long next_Pos = X + 10000;
                go_to(1, lu_to_string(next_Pos), false);
            }

            else if (c == 'S')
            {
                unsigned long next_Pos = X - 10000;
                go_to(1, lu_to_string(next_Pos), false);
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
        if (verbose)
            printf("CONTROLLER_DEBUG(parse_COMMAND: Target(char): %s\n", lu_to_string(target));
        go_to(channel, lu_to_string(target), false);
    }
    else if (strcasecmp(input, "exit") == 0 || strcasecmp(input, "quit") == 0)
    {
        shutdown_Controller(port);
        exit(1);
    }
    else if (strcasecmp(input, "scan") == 0)
    {
        char range[MAX_INPUT];
        char field[MAX_INPUT];
        printf("\nEnter link range (meters):\t");
        scanf("%s", range);
        printf("\nEnter link field (degrees):\t");
        scanf("%s", field);

        unsigned long range_lu = strtol(range, NULL, 10);
        double field_d = strtod(field, NULL);
        scan(range_lu, field_d);
    }
    else if (strcasecmp(input, "feedback") == 0)
    {
        PID_controller();
    }
    else
        parse_Response((send_Command(input)));
}

void wait_For_Input()
{
    char input[MAX_INPUT];
    while (true)
    {
        printf("Command:");
        scanf("%s", input);
        parse_Command(input);
    }
}

int main(int argc, char **argv)
{
    system("clear");
    if (argc > 1 && strcasecmp(argv[1], "verbose") == 0)
    {
        verbose = 1;
        printf("\nVerbose Mode: On\n");
        printf("Comms: Version: %.2f\n", VERSION_COMMS);
        printf("Driver: Version: %.2f\n", VERSION_DRIVER);
        printf("Initialisation: Version: %.2f\n", VERSION_INITIALISATION);
        printf("SlowFeedback: Version: %.2f\n", VERSION_SLOW_FEEDBACK);
        printf("Controller: Version: %.2f\n", VERSION_CONTROLLER);
    }
    port = begin_Comms();
    wait_For_Input();
}
