/*--------------------------------------------------------------
* EQ8 - Controller
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 12/03/2020
* Last modifiied on: 13/08/2020
* 
* This library contains high level mount controlling functionality:
*   
*-------------------------------------------------------------*/
#define VERSION_CONTROLLER 2.14
#define STEPS_PER_REV_CHANNEL 11136000

#include "Driver.c"
#include "system_calls.c"
#include "Initialisation.c"
#include "SlowFeedback.c"
#include "stabilisation.c"
int initial_X_position = -1;
int initial_Y_position = -1;

/* *
 * Function to interpret keyboard commands.
 * Allows for command strings to be defined for more complex,
 * or series of commands
 * */
void help(int option)
{
    if (option == 1)
    {
        char *helpstring = "usage: ./Controller [-v verbose mode] [command]\n"
                           "[command] initiates the controller and immediately calls the provided command\n\n"
                           "Commands:\n"
                           "position\tContinuously prints position of mount axes.\n"
                           "manual\t\tAllows user to manually control position of mount through joystick or keyboard.\n"
                           "go\t\tTurns axis to a given target (encoder) position. \n"
                           "turn\t\tMoves mount a given number of degrees.\n"
                           "scan\t\tStarts initialisation scan.\n"
                           "feedback\tStarts slow-feedback program.\n"
                           "stabilisation\tRuns stabilisation program\n"
                           "exit\t\tSevers port connection and quits program.\n\n"
                           "\nAll other inputs are interpreted as commands and are sent to the mount.\n\n";
        printf("%s", helpstring);
    }
}

void options(char c)
{
    if (c == 'v')
    {
        verbose = 1;
        printf("\nVerbose Mode: On\n");
        printf("Comms: Version: %.2f\n", VERSION_COMMS);
        printf("Driver: Version: %.2f\n", VERSION_DRIVER);
        printf("Initialisation: Version: %.2f\n", VERSION_INITIALISATION);
        printf("SlowFeedback: Version: %.2f\n", VERSION_SLOW_FEEDBACK);
        printf("Controller: Version: %.2f\n", VERSION_CONTROLLER);
    }
    if (c == 'c')
    {
        printf("Comms mode: off\nNo commands will be sent to the mount\n");
        comms = 0;
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
            else
                break;

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
        float angle_float = atof(angle);
        if (angle_float > 10)
        {
            printf("\nAngle too big\n");
            return;
        }
        turn(channel, angle_float);
    }
    else if (strcasecmp(input, "exit") == 0 || strcasecmp(input, "quit") == 0)
    {
        stop_channel(3);
        shutdown_Controller(port);
        exit(1);
    }
    else if (strcasecmp(input, "scan") == 0)
    {
        get_Scan_Parameters();
    }
    else if (strcasecmp(input, "feedback") == 0)
    {
        // PID_controller();
    }
    else if (strcasecmp(input, "stabilisation") == 0)
    {
        stabilisation(30000);
    }
    else if (strcasecmp(input, "tab") == 0) //meme command
    {
        char input2[MAX_INPUT];
        printf("CHECK CORE TEMPERATURE?\nYES/NO\n");
        scanf("%s", input2);
        if (strcasecmp(input2, "YES") == 0)
            printf("CORE TEMPERATURE NORMAL\n");
        else
            return;
    VENT:
        printf("\nVENT RADIOACTIVE GAS?\nYES/NO\n");

        scanf("%s", input2);
        if (strcasecmp(input2, "NO") == 0)
        {
            printf("VENTING PREVENTS EXPLOSION\n");
            goto VENT;
        }

        else if (strcasecmp(input2, "YES") == 0)
            printf("VENTING RADIOACTIVE GAS\n");
    }
    else
        parse_Response((send_Command(input)));
}

void wait_For_Input()
{
    char input[MAX_INPUT];
    while (true)
    {
        printf("\nCommand:");
        scanf("%s", input);
        parse_Command(input);
    }
}

int main(int argc, char **argv)
{
    system("clear");
    int command = 0;
    if (argc > 1) //if CLI arguments are provided
    {
        for (int i = 1; i < argc; i++) //for each argument
            if (argv[i][0] == '-')     //is it an option ("-*")?
                options(argv[i][1]);
            else //its a command
                command = i;
    }

    setup();
    if (comms)
        port = setup_Port();
    initial_X_position = get_Position(2);
    initial_Y_position = get_Position(1);

    if (initial_Y_position == -1 || initial_X_position == -1)
        {
            printf("Error obtaining initial position from mount.\n Check connection and power.\n Exiting...\n");
        }
    
    if (command) //if a CLI command was provided
        parse_Command(argv[command]);
    wait_For_Input();
}
