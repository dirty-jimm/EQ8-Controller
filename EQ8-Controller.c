/*--------------------------------------------------------------
* EQ8 - Controller
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 12/03/2020
* Last modifiied on: 12/03/2020
* 
* This library contains high level mount controlling functionality:
*   
* This should be the highest-level library common to all controllers.
*-------------------------------------------------------------*/
#define VERSION_CONTROLLER 1.8
#include "EQ8-Driver.c"
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
        printf("Position Mode: \nPress 'c' to cancel\n");
        char data[7];
        system("/bin/stty raw");
        char command[3] = "j1";
        do
        {   
            command[1] = '2';
            convert_Response((*send_Command(command)).data, data);
            printf("\r1: %s, ", data);

            command[1] = '1';
            convert_Response((*send_Command(command)).data, data);
            printf("2: %s", data);
            
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
        char target[6];
        printf("\nEnter target:\t");
        scanf("%s", target);
        go_to(1, target, false);
    }

    else if (strcasecmp(input, "go2") == 0)
    {
        char target[6];
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
        // if (verbose)
        printf("Target: %06lX\n", target);

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
        printf("\nCommand:");
        scanf("%s", input);
        parse_Command(input);
    }
}
