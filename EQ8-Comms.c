/*--------------------------------------------------------------
* EQ8 - Comms
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 11/02/2020
* Last modifiied on: 11/03/2020
*
*
* This library allows for low level communication with the motor 
* controllers through a USB - RJ45 Serial connection.
*
* No logical-error checking functionality is provided here (for instance,
* attempting to set the GOTO location while the mount is moving).
* Each function will return a negative value on failure, positive on success
* (for recieve, this is done so as the .flag value in the returned structure)
*-------------------------------------------------------------*/
#define VERSION_COMMS 1.6

#include <stdio.h>
#include <string.h>
#include <unistd.h> //Linux requires this
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>   /* File Control Definitions           */
#include <termios.h> /* POSIX Terminal Control Definitions */
#include <unistd.h>  /* UNIX Standard Definitions      */
#include <errno.h>   /* ERROR Number Definitions           */


//#define PORT "/dev/cu.usbserial-00002014" // Port mount is connected through, Mac
#define PORT "/dev/serial/by-id/usb-FTDI_USB__-__Serial-if00-port0" // Linux
//#define PORT "/dev/serial/by-id/usb-1a86_USB2.0-Serial-if00-port0" //Linux
#define MAC_LS "ls -1a /dev/cu.usb*"
#define LINUX_LS "ls -1a /dev/serial/by-id/"

bool verbose = 0;                                                   //Enables verbose terminal output for debugging

/* *
 * Structure used to return both a success flag and data
 * */
struct response
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
        printf("Search serial ports? Y/N ");
        
        char c = getchar(); 
        if (c == 'Y' || c == 'y')
        {
            printf("Available USB Serial Ports:\n");
            system(MAC_LS);
            //system(LINUX_LS);
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
    /* 8N1 Mode */
    SerialPortSettings.c_cflag &= ~PARENB;                        /* Disables the Parity Enable bit(PARENB),So No Parity   */
    SerialPortSettings.c_cflag &= ~CSTOPB;                        /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
    SerialPortSettings.c_cflag &= ~CSIZE;                         /* Clears the mask for setting the data size             */
    SerialPortSettings.c_cflag |= CS8;                            /* Set the data bits = 8                                 */
    SerialPortSettings.c_cflag &= ~CRTSCTS;                       /* No Hardware flow Control                         */
    SerialPortSettings.c_cflag |= CREAD | CLOCAL;                 /* Enable receiver, Ignore Modem Control lines       */
    SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);        /* Disable XON/XOFF flow control both i/p and o/p */
    SerialPortSettings.c_iflag &= (ICANON | ECHO | ECHOE | ISIG); /* Non Cannonical mode                            */
    SerialPortSettings.c_oflag &= ~OPOST;                         /*No Output Processing*/
    /* Setting Time outs */
    SerialPortSettings.c_cc[VMIN] = 8; /* Read at least 10 characters */
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
    strcpy(writebuffer, ":"); //this was originally strcat, cause memory leak on linux (not on Mac for some reason)
    strcat(writebuffer, command);
    strcat(writebuffer, "\r");

    if (verbose)
        printf("COMMS_DEBUG(TX) Command: %s\n", writebuffer);

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
    char read_buffer[9]; //Expecting 8 bytes as per tech document, eg. =123456\r
    //not sure why but gets errors if less than 9 in buffer, might be \0?
    int bytes_read = 0;
    static struct response this_response = {-1};
    strcpy(this_response.data, "\0"); // flush previous read

    bytes_read = read(port, &read_buffer, 8); /* Read the data */
    tcflush(port, TCIFLUSH);                  /* Flush RX buffer */

    if (read_buffer[0] == '=' || read_buffer[0] == '!') //valid response from mount
        this_response.flag = 1;

    strcpy(this_response.data, read_buffer);
    if (verbose)
        printf("COMMS_DEBUG(RX): %i Bytes recieved, %s\n", bytes_read, read_buffer);
    
    return &this_response;
}
