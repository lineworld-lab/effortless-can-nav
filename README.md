1. Using ecsh 
ecsh_help: Displays the help message.

ecsh_vendor: Downloads and builds the external libraries SOEM and cJSON.

ecsh_build: Deletes the existing build if it exists, creates a new build folder, and builds with CMake.

ecsh_vendor_rt: Installs the real-time library EtherCAT.

ecsh_build_rt: Performs the real-time build.

ecsh_run: Executes the program.

When you run ./ecsh help, ./ecsh vendor, or ./ecsh build, it executes according to each corresponding command.


2. Config Modification

When using the ifconfig command from the raspi account, the IP address can be confirmed as 192.168.50.177 9999. After running cat ecctl, the content was not understandable, so I installed hexedit with sudo apt install hexedit and used hexedit ecctl to identify that the file is a binary file and to check its contents.

3. Using ecctl

When using cat ecctl, the content was not understandable, so I installed hexedit with sudo apt install hexedit and used hexedit ecctl to identify that the file is a binary file and to check its contents.



























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

