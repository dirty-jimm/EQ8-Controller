/*--------------------------------------------------------------
* EQ8 - Comms
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 11/02/2020
* Last modifiied on: 13/02/2020
*
*
* This library allows for low level communication with the motor 
* controllers through a USB - RJ45 Serial connection.
* Though designated as a driver, this program is not one by strict definition,
* but rather it exists to provide the same functionality as a driver would.
* It largely complies (with a few execptions with the general principles dictated 
* by ASCOM found at: https://ascom-standards.org/Developer/Principles.htm
*
* No logical-error checking functionality is provided here (for instance,
* attempting to set the GOTO location while the mount is moving).
* Each function will return a negative value on failure, positive on success
* (for recieve, this is done so as the .flag value in the returned structure)
*-------------------------------------------------------------*/
#define VERSION_COMMS 1.4

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>                        /* File Control Definitions           */
#include <termios.h>                      /* POSIX Terminal Control Definitions */
#include <unistd.h>                       /* UNIX Standard Definitions      */
#include <errno.h>                        /* ERROR Number Definitions           */
#define PORT "/dev/cu.usbserial-00001014" // Port mount is connected through

bool verbose = 0; //Enables verbose terminal output for debugging

struct response //Structure used to return both a success flag and data
{
    int flag;      // Response flag, negative = error
    char data[10]; // data
};

/* *
 * Function to initialise port settings and open connection.
 * Returns port ID on success, -1 on failure.
 * */
int setup_Port()
{
    if (verbose)
        printf("Verbose Mode: on\n");
    printf("\nEQ8 Pro Mount Comms:\n");

    if (verbose)
        printf("Version: %.2f\n", VERSION_COMMS);

    int fd = open(PORT, O_RDWR | O_NOCTTY | O_NDELAY); /* ttyUSB0 is the FT232 based USB2SERIAL Converter   */
                                                       //  fd = open("/dev/ttyUSB0",O_RDWR | O_NOCTTY | O_NDELAY); /* ttyUSB0 is the FT232 based USB2SERIAL Converter   */
                                                       /* O_RDWR   - Read/Write access to serial port       */
                                                       /* O_NOCTTY - No terminal will control the process   */
                                                       /* Open in blocking mode,read will wait              */

    if (fd == -1)
    {
        printf("Port Error\nCould not find mount on port: %s\nCheck connection\n", PORT);
        exit(-1);
    }
    else
        printf("Connected to Mount, port: %s\n", PORT);

    struct termios SerialPortSettings;       /* Create the structure                          */
    tcgetattr(fd, &SerialPortSettings);      /* Get the current attributes of the Serial port */
    cfsetispeed(&SerialPortSettings, B9600); /* Set Read  Speed as 9600                       */
    cfsetospeed(&SerialPortSettings, B9600); /* Set Write Speed as 9600                      */
    /* 8N1 Mode */
    SerialPortSettings.c_cflag &= ~PARENB;                        /* Disables the Parity Enable bit(PARENB),So No Parity   */
    SerialPortSettings.c_cflag &= ~CSTOPB;                        /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
    SerialPortSettings.c_cflag &= ~CSIZE;                         /* Clears the mask for setting the data size             */
    SerialPortSettings.c_cflag |= CS8;                            /* Set the data bits = 8                                 */
    SerialPortSettings.c_cflag &= ~CRTSCTS;                       /* No Hardware flow Control                         */
    SerialPortSettings.c_cflag |= CREAD | CLOCAL;                 /* Enable receiver,Ignore Modem Control lines       */
    SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);        /* Disable XON/XOFF flow control both i/p and o/p */
    SerialPortSettings.c_iflag &= (ICANON | ECHO | ECHOE | ISIG); /* Non Cannonical mode                            */
    SerialPortSettings.c_oflag &= ~OPOST;                         /*No Output Processing*/
    /* Setting Time outs */
    SerialPortSettings.c_cc[VMIN] = 13; /* Read at least 10 characters */
    SerialPortSettings.c_cc[VTIME] = 0; /* Wait indefinetly   */

    if ((tcsetattr(fd, TCSANOW, &SerialPortSettings)) != 0) /* Set the attributes to the termios structure*/
        printf("ERROR ! in Setting attributes\n");
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
int TX(int port, char command[])
{
    char writebuffer[strlen(command) + 2]; // Create buffer with length of the command +2 for the leading ":" and trailing RC
    strcat(writebuffer, ":");
    strcat(writebuffer, command);
    strcat(writebuffer, "\r");

    if (verbose)
        printf("Writing: %s\n", writebuffer);
    int X = write(port, writebuffer, strlen(writebuffer));
    if (verbose)
        printf("COMMS_DEBUG: %i bytes written\n", X);
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
    char read_buffer[9]; //Expecting 8 bytes as per tech document,
    //not sure why but get fatal errors if less than 9 in buffer
    int bytes_read = 0;
    static struct response this_response = {-1};
    strcpy(this_response.data, "\0"); // flush previous read

    bytes_read = read(port, &read_buffer, 8); /* Read the data */
    tcflush(port, TCIFLUSH);                  /* Flush RX buffer */

    if (read_buffer[0] == '=' || read_buffer[0] == '!') //valid response from mount
        this_response.flag = 1;

    strcpy(this_response.data, read_buffer);
    if (verbose)
    {
        printf("COMMS_DEBUG: %i Bytes recieved, ", bytes_read);
        printf("%s\n", read_buffer);
        printf("COMMS_DEBUG: Written to struct: Flag: %i\n", this_response.flag);
        printf("COMMS_DEBUG: Written to struct: Data: %s\n", this_response.data);
    }
    return &this_response;
}