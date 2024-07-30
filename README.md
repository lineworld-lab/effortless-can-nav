1. Using ecsh 
./ecsh help

./ecsh vendor
ㅁ Downloads and builds the external libraries SOEM and cJSON.

./ecsh build
ㅁ The ecserver program is ready to run. You can use the ./ecsh run command to execute it.

./ecsh vendor rt: Installs the real-time library EtherCAT.

./ecsh build rt: Performs the real-time build.

./ecsh run: Executes the program.
ㅁ The SOEM library is started, and this library functions as a master. The servo controller is initialized, and the home sensor information for each motor and the start offset values are displayed.


2. Config Modification
If the name in if_name is incorrect, it should be corrected by checking the interface name using ifconfig or ip a.
In drivers_num, verify whether it matches the actual number of connected slaves, and adjust the value if it differs.
In homing_at_start, if there is no need to automatically move to the home position or if issues arise, change the value from 1 to 0 to disable the functionality.


3. Using ecctl

./ecctl -> connect 192.168.50.177 9999 -> send -> axis tmo params
By entering the command ./ecctl -> connect 192.168.50.177 9999 -> send -> axis tmo params, you can adjust the parameters of the desired wheel.

.























+ off topic
int RunInteractive() {  } 
// The variable 'keep' is initialized to 0, and the loop runs until this condition is true.

while (keep == 0) {  } 
// An infinite loop executes until a command is entered.

fgets(line, MAX_BUFFLEN, stdin); 
// Use the fgets function to read input, and store the content in the 'line' array.

int line_len = strlen(line); 
// Check the length of the input string.

if (line[i] == '\n')  
    line[i] = '\0'; 
	// Replace the newline character (\n) with a null character (\0), indicating the end of the string.


char* tok = strtok(line, delim); 
// Use the strtok function to tokenize the input string based on the delimiter (" ").

if (arg_len == 0) ~ else 
// Allocate memory using malloc for the first token, and use realloc for reallocating memory for subsequent tokens.

if (strcmp(runtime_args[0], "connect") == 0) 
// If ret_code is less than 0, print a failure message.

else if (strcmp(runtime_args[0], "send") == 0) 
// After entering the send command, transmit data. The 'res' array stores the transmission 

results and prints messages based on the transmission status.
else if (strcmp(runtime_args[0], "disconnect") == 0)  
// Terminate the connection.

else  
// If the command does not match, print a message indicating an invalid argument.


int ConnectTo(char* res, int arg_len, char** runtime_args) 
    if (arg_len < 3) 
    // If the number of arguments is less than 3, print an error message and return -1. The required arguments are address and port.
    
    
ret_code = InitECCmdGatewayWithTimeout(res, addr, port, 5000); 
// Connect using the address and port.

