/*--------------------------------------------------------------
* EQ8 - Driver
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 05/02/2020
* Last modifiied on: 07/02/2020
*
*
* This program allows for low level communication with the motor 
* controllers through a USB - RJ45 Serial connection.
* Though designated as a driver, this program is not one by strict definition,
* but rather it exists to provide the same functionality as a driver would.
* It largely complies (with a few execptions with the general principles dictated 
* by ASCOM found at: https://ascom-standards.org/Developer/Principles.htm
*
* No logical-error checking functionality is provided here (for instance,
* attempting to set the GOTO location while the mount is moving). Rather,
* every function will return either data or an error flag. 
*
*-------------------------------------------------------------*/
#define VERSION 1.2

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>                        /* File Control Definitions           */
#include <termios.h>                      /* POSIX Terminal Control Definitions */
#include <unistd.h>                       /* UNIX Standard Definitions      */
#include <errno.h>                        /* ERROR Number Definitions           */
#define PORT "/dev/cu.usbserial-00001014" // Port mount is connected through

bool verbose = 1; //Enables verbose terminal output for debugging

/* *
 * Function to initialise port settings and open connection.
 * Returns port ID on success, -1 on failure.
 * */
int setup_Port()
{

    int fd = open(PORT, O_RDWR | O_NOCTTY | O_NDELAY); /* ttyUSB0 is the FT232 based USB2SERIAL Converter   */
                                                       //  fd = open("/dev/ttyUSB0",O_RDWR | O_NOCTTY | O_NDELAY); /* ttyUSB0 is the FT232 based USB2SERIAL Converter   */
                                                       /* O_RDWR   - Read/Write access to serial port       */
                                                       /* O_NOCTTY - No terminal will control the process   */
                                                       /* Open in blocking mode,read will wait              */

    if (fd == -1) /* Error Checking */
    {
        printf("Port Error\nCould not find mount on port: %s\nCheck connection\n", PORT);
    }
    else if (verbose)
        printf("Connected to Mount, port: %s\n", PORT);

    /*---------- Setting the Attributes of the serial port using termios structure --------- */

    struct termios SerialPortSettings; /* Create the structure                          */

    tcgetattr(fd, &SerialPortSettings); /* Get the current attributes of the Serial port */

    /* Setting the Baud rate */
    cfsetispeed(&SerialPortSettings, B9600); /* Set Read  Speed as 9600                       */
    cfsetospeed(&SerialPortSettings, B9600); /* Set Write Speed as 9600                      */

    /* 8N1 Mode */
    SerialPortSettings.c_cflag &= ~PARENB; /* Disables the Parity Enable bit(PARENB),So No Parity   */
    SerialPortSettings.c_cflag &= ~CSTOPB; /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
    SerialPortSettings.c_cflag &= ~CSIZE;  /* Clears the mask for setting the data size             */
    SerialPortSettings.c_cflag |= CS8;     /* Set the data bits = 8                                 */

    SerialPortSettings.c_cflag &= ~CRTSCTS;       /* No Hardware flow Control                         */
    SerialPortSettings.c_cflag |= CREAD | CLOCAL; /* Enable receiver,Ignore Modem Control lines       */

    SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);         /* Disable XON/XOFF flow control both i/p and o/p */
    SerialPortSettings.c_iflag &= (ICANON | ECHO | ECHOE | ISIG); /* Non Cannonical mode                            */

    SerialPortSettings.c_oflag &= ~OPOST; /*No Output Processing*/

    /* Setting Time outs */
    SerialPortSettings.c_cc[VMIN] = 13; /* Read at least 10 characters */
    SerialPortSettings.c_cc[VTIME] = 0; /* Wait indefinetly   */

    if ((tcsetattr(fd, TCSANOW, &SerialPortSettings)) != 0) /* Set the attributes to the termios structure*/
        printf("ERROR ! in Setting attributes\n");
    else if (verbose)
        printf("BaudRate = 9600 \nStopBits = 1 \nParity   = none\n");
    return fd;
}

/* *
 * Function to send a single command to the mount.
 * ":" prefix and "\r" suffix are padded automatically (improves reliability),
 * and should be ommitted from the "command" argument.
 * Returns number of bytes sent on success, -1 on failure.
 * */
int send_Command(int port, char command[])
{
    char writebuffer[strlen(command) + 2]; // Create buffer with length of the command +2 for the leading ":" and trailing RC
    strcat(writebuffer, ":");
    strcat(writebuffer, command);
    strcat(writebuffer, "\r");

    if (verbose)
        printf("\nWriting: %s\n", writebuffer);
    int X = write(port, writebuffer, strlen(writebuffer));
    if (verbose)
        printf("DEBUG: %i bytes written\n", X);
    return X;
}

int read_Response(int port)
{
    char read_buffer[8];
    int bytes_read = 0;
    do
    {
        bytes_read = read(port, &read_buffer, 8); /* Read the data                   */
    } while (bytes_read == 0);
    if (verbose)
    {
        printf("DEBUG: %i Bytes recieved\n", bytes_read); /* Print the number of bytes read */

        if (bytes_read > 0)
        {
            for (int i = 0; i < bytes_read - 1; i++) /*printing only the needed bytes*/
                printf("%c", read_buffer[i]);
            printf("\n");
        }
    }
    tcflush(port, TCIFLUSH); /* Discards old data in the rx buffer            */
    return bytes_read;
}

int main(void)
{
    printf("\n\n\n--- EQ8 Pro Mount Driver ---\n");
    if (verbose)
        printf("Verbose Mode: on\n");
    if (verbose)
        printf("Version: %.2f\n", VERSION);
    int fd = setup_Port();
    printf("\nSetup Complete\n\n");
    char input[32];
    
    //Test loop to allow keyboard commands, to be removed
    while (1)
    {
        printf("Command:");
        scanf("%s", input);
        send_Command(fd, input);
        usleep(30000); // this gives the mount time to repsond
        // else all responsed will be offset by 1 from their respective commands
        read_Response(fd);
    }

    printf("\n +----------------------------------+\n\n\n");

    close(fd); /* Close the serial port */
}