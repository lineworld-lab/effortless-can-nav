1. Using ecsh 
./ecsh help

./ecsh vendor
ㅁ Downloads and builds the external libraries SOEM and cJSON.

./ecsh build
ㅁ The ecserver program is ready to run. You can use the ./ecsh run command to execute it.

./ecsh vendor_rt: Installs the real-time library EtherCAT.

./ecsh build_rt: Performs the real-time build.

./ecsh run: Executes the program.
ㅁ The SOEM library is started, and this library functions as a master. The servo controller is initialized, and the home sensor information for each motor and the start offset values are displayed.


2. Config Modification
If the name in if_name is incorrect, it should be corrected by checking the interface name using ifconfig or ip a.
In drivers_num, verify whether it matches the actual number of connected slaves, and adjust the value if it differs.
In homing_at_start, if there is no need to automatically move to the home position or if issues arise, change the value from 1 to 0 to disable the functionality.


3. Using ecctl

./ecctl -> connect IP PORT -> send -> axis tmo params
By entering the command ./ecctl -> connect 192.168.50.177 9999 -> send -> axis tmo params, you can adjust the parameters of the desired wheel.

