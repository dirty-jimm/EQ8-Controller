/*--------------------------------------------------------------
* EQ8 - Comms
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 11/02/2020
* Last modifiied on: 12/05/2020
*

* This library allows for low level communication with the motor 
* controllers through a USB - RJ45 Serial connection.
*-------------------------------------------------------------*/
#define VERSION_COMMS 2.1

#include <stdio.h>
#include <string.h>
#include <unistd.h> //Linux requires this
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>   /* File Control Definitions           */
#include <termios.h> /* POSIX Terminal Control Definitions */
#include <unistd.h>  /* UNIX Standard Definitions      */
#include <errno.h>   /* ERROR Number Definitions           */

//#define PORT "/dev/cu.usbserial-00001014" // Port mount is connected through, Mac
//#define PORT "/dev/cu.usbserial-000020" // Port mount is connected through, Mac (Alt)
#define PORT "/dev/serial/by-id/usb-FTDI_USB__-__Serial-if00-port0" // Linux
#define LINUX_LS "ls -1a /dev/serial/by-id/"

bool verbose = 0; //Enables verbose terminal output for debugging

/* *
 * Structure used to return both a success flag and data
 * */
struct response
{
    int flag;     // Response flag, negative = error
    char data[6]; // data
};

/* *
 * Function to initialise port settings and open connection.
 * Returns port ID on success, -1 on failure.
 * */
int setup_Port()
{
    if (verbose)
    {
        printf("Verbose Mode: on\n");
        printf("\nEQ8 Pro Mount Comms:\n");
        printf("Version: %.2f\n", VERSION_COMMS);
    }

    int fd = open(PORT, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1)
    {
        printf("Port Error\nCould not find mount on port: %s\nCheck connection\n", PORT);
        printf("Search serial ports? Y/N ");

        char c = getchar();
        if (c == 'Y' || c == 'y')
        {
            printf("Available USB Serial Ports:\n");
            system(LINUX_LS);
            printf("\n");
        }
        exit(-1);
    }
    else
        printf("Connected to Mount, port: %s\n", PORT);

    struct termios SerialPortSettings;       /* Create the structure                          */
    tcgetattr(fd, &SerialPortSettings);      /* Get the current attributes of the Serial port */
    cfsetispeed(&SerialPortSettings, B9600); /* Set Read  Speed as 9600                       */
    cfsetospeed(&SerialPortSettings, B9600); /* Set Write Speed as 9600                      */

    //Control Flags
    SerialPortSettings.c_cflag &= ~PARENB;        /* Disables the Parity Enable bit(PARENB),So No Parity   */
    SerialPortSettings.c_cflag &= ~CSTOPB;        /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
    SerialPortSettings.c_cflag &= ~CSIZE;         /* Clears the mask for setting the data size             */
    SerialPortSettings.c_cflag |= CS8;            /* Set the data bits = 8                                 */
    SerialPortSettings.c_cflag &= ~CRTSCTS;       /* No Hardware flow Control                         */
    SerialPortSettings.c_cflag |= CREAD | CLOCAL; /* Enable receiver,Ignore Modem Control lines       */
    //Input Flags
    SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);        /* Disable XON/XOFF flow control both i/p and o/p */
    SerialPortSettings.c_lflag &= (ICANON | ECHO | ECHOE | ISIG); /* Non Cannonical mode                            */
    //Output Flags
    SerialPortSettings.c_oflag &= ~OPOST; /*No Output Processing*/
    //Control Characters
    SerialPortSettings.c_cc[VMIN] = 13; /* Read at least 10 characters */
    SerialPortSettings.c_cc[VTIME] = 0; /* Wait indefinetly   */

    if ((tcsetattr(fd, TCSANOW, &SerialPortSettings)) != 0) /* Set the attributes to the termios structure*/
        printf("Error in Setting attributes\n");
    else if (verbose)
        printf("BaudRate = 9600 \nStopBits = 1 \nParity   = none\n");

    printf("Comms Setup Complete\n\n");
    return fd;
}

/* *
 * Function to transmit a single command to the mount.
 * ":" prefix and "\r" suffix are padded automatically (improves reliability),
 * and should be ommitted from the "command" argument.
 * Returns number of bytes sent on success, -1 on failure.
 * */
int TX(int port, char writebuffer[])
{
    if (verbose)
    {
        printf("COMMS_DEBUG(TX) Command: %s\n", writebuffer);
        printf("COMMS_DEBUG(TX) Writing %zu bytes\n", strlen(writebuffer));
    }
    int X = write(port, writebuffer, strlen(writebuffer));
    if (verbose)
        printf("COMMS_DEBUG(TX): %i bytes written\n", X);
    return X;
}

/* *
 * Function to read a single response from the mount.
 * Populates a structure and returns a pointer to it.
 * Structure is cleared on and only on calling of this
 * function.
 * */
struct response *RX(int port)
{
    static struct response this_response = {-1};
    memset(this_response.data, 0, strlen(this_response.data)); // flush previous read

    int bytes_read = read(port, this_response.data, 8); /* Read the data */
    tcflush(port, TCIFLUSH);                            /* Flush RX buffer */

    if (this_response.data[0] == '=') //valid response from mount
        this_response.flag = 1;

    if (this_response.data[0] == '!')
        this_response.flag = 2;
        
        if (verbose)
        {
            printf("COMMS_DEBUG(RX): %i Bytes recieved\n", bytes_read);
            printf("COMMS_DEBUG(RX): Response: %s\n", this_response.data);
        }
    return &this_response;
}
