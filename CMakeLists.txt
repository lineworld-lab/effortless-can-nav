cmake_minimum_required(VERSION 3.5)
project(ecat_pkg)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

add_compile_options(-g -w -Wall -Wextra -Wpedantic)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

## Defining paths and libraries to include in the next section.

if(ECRT)

  set(etherlab_include vendor/ethercat/include)
  set(etherlab_lib vendor/ethercat/lib/.libs)

else()

  set(etherlab_include 
      
    vendor/SOEM
    vendor/SOEM/soem 
    vendor/SOEM/osal
    vendor/SOEM/osal/linux 
    vendor/SOEM/oshw
    vendor/SOEM/oshw/linux
        
        )
  set(etherlab_lib vendor/SOEM/build)

endif()

set(cjson_include vendor/cJSON)

set(ecat_node_include ${CMAKE_CURRENT_SOURCE_DIR}/include/)
set(node_name "ecat_node")


if(ECRT)

  file(GLOB ecat_node_SRC
      "src/ec-server/v1/*.c"
  )


else()

  file(GLOB ecat_node_SRC
      "src/ec-server/v2/*.c"
      "src/ec-server/wheel/*.c"
  )


endif()



file(GLOB ecat_req_SRC
    "src/ec-client/v1/request/*.c"
)

file(GLOB ecat_cli_SRC
    "src/ec-client/v1/cmd/*.c"
)

file(GLOB cjson_SRC
    "vendor/cJSON/cJSON.c"
    "vendor/cJSON/cJSON_Utils.c"
)


# Add include directories
include_directories(
  ${etherlab_include}
  ${cjson_include}
  ${ecat_node_include}
)


link_directories(
  ${etherlab_lib}
)

add_library(ecrequest
  SHARED
  ${ecat_req_SRC}
)

add_library(cjson
  SHARED
  ${cjson_SRC}
)

add_executable(ecctl ${ecat_cli_SRC})


add_executable(ecserver ${ecat_node_SRC})



## Specifying libraries by using definitions above.

target_link_libraries(
  ecctl
  PRIVATE
  ecrequest
)


if(ECRT)
  target_link_libraries(
    ecserver
    PRIVATE
    ethercat
    cjson 
    Threads::Threads
    rt
  )


else()

  target_link_libraries(
    ecserver
    PRIVATE
    soem
    cjson 
    Threads::Threads
    rt
  )

endif()



