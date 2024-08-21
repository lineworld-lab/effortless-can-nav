# effortless-can-nav

life is too short for hassling over re-inventing CAN wheels (literally)


## showcase


This is the actual test drive on FASTECH Ezi-Servo-PR-60M-A and AMPS TECH L2DB-4830-CAFC


[navigate.webm](https://github.com/user-attachments/assets/589b00a8-3f41-4ae4-917d-4f53a48ea034)


## How to 

1. Using ecsh 

```shell

########################
#  ec-server/v2        #
########################

# help
./ecsh help


# downloads and builds the external libraries SOEM, CANopenLinux, cJSON.
./ecsh vendor


# this will build the binaries needed to use this project

./ecsh build

# starts the server based on your config
# see below [ 2. Config Modification ]

./ecsh server 

# starts the command-line client to interact with the server
# see below [ 3. Using Command-line client ]

./ecsh client

###########################
# ec-server/v1 DEPRECATED #
###########################

# installs the real-time library IgH etherlab.

./ecsh vendor_rt

# builds based on IgH etherlab

./ecsh build_rt

```



2. Config Modification

Sample configuration is ./config/config.json

At this moment, configuration looks like below

```json

{

    "drive": {

        "drivers_num": 4,
        "drivers":[
    
            {
                "id": 0,
                "homing_offset": 332000,
                "start_offset":0
            },
            {
                "id": 1,
                "homing_offset": 300000,
                "start_offset":0
            },
            {
                "id": 2,
                "homing_offset": 393000,
                "start_offset":0
            },
            {
                "id": 3,
                "homing_offset": 139000,
                "start_offset":0
            }
    
        ],
        "if_name": "enx00e04c3607cc",
        "homing_margin_degrees":5,
        "velocity_to_start_offset": 100000,
        "move_fail_threshold":50,
        "working_counter_fail_threshold": 20,
        "error_count_threshold": 10,
        "debug_mode": 0,
        "homing_at_start": 0
    },

    "wheel":{
        "wheels_num": 4,
        "wheels":[
    
            {
                "id": 1,
                "rotation_clockwise": 1
            },
            {
                "id": 2,
                "rotation_clockwise": 0
            },
            {
                "id": 3,
                "rotation_clockwise": 1
            },
            {
                "id": 4,
                "rotation_clockwise": 0
            }
    
        ],
        "master_node_id": "77",
        "if_name": "can0",
        "local_socket_name": "/tmp/EC_wheel_sock",
        "acceleration": 100,
        "deceleration": 100
    }

    
}


```


3. Using Command-line client

```shell

# started by ./ecsh client
# you will be prompted with something like below

CMD: 

# then, type this to connect to server

CMD: $IP $PORT

# $IP is your machine's ip
# $PORT is 9999 by default

# run below to get help

CMD: help

# run below to see what commands are available

CMD: discovery

```
