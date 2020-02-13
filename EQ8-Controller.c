/*--------------------------------------------------------------
* Mount Controller
* Author: Jim Leipold
* Email: james.leipold@hotmail.com
* Created on: 11/02/2020
* Last modifiied on: 13/02/2020
*
* This program contains high level mount controlling functionality:
*   - Parsing of text commands.
    - Parsing of error codes
*   - Command retries and logical error handling.
*   - 
*-------------------------------------------------------------*/

#define VERSION_CONTROLLER 1.4
#include "EQ8-Driver.c"
int main(void)
{
  char input[32];
  int fd = setup_Controller();

  while (true)
  {
    printf("\nCommand:\t");
    scanf("%s", input);
    send_Command(fd, input);
  }
  //if (strcmp(input, "TEST")==0)

  /*TEST:
    strcpy(input, "j1");
  

    GOTO:*/
}
