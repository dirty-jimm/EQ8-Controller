
#define STEPS_PER_REV_CHANNEL 0xA9EC00
#define MAX_INPUT 128
#include <stdbool.h>

int setup_Port();
int TX(int port, char writebuffer[]);
struct response *RX(int port);
extern bool verbose;
int kbhit();
int begin_Comms(void);
int shutdown_Controller(int port;
int convert_Command(char data_in[10], char data_out[8]);
int convert_Response(char data_in[8], char data_out[8]);
int parse_Response(struct response *response);
struct response *send_Command(char command[]);
int get_Status(int channel);
unsigned long angle_to_argument(int channel, double angle);
char *lu_to_string(unsigned long input);
int go_to(int channel, char target[MAX_INPUT], bool isFormatted);
unsigned long get_Position(int channel);

void parse_Command(char input[MAX_INPUT]);
void wait_For_Input();
int main(int argc, char **argv);**/