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
## Compile options for IgH libary and several Linux libraries (e.g lpthread)
add_compile_options(-g -w -Wall -Wextra -Wpedantic)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

## Defining paths and libraries to include in the next section.
if(BUILD_DEB64)
  set(etherlab_include ethercat/include)
  set(etherlab_lib ethercat/lib/.libs)
elseif(BUILD_JETSON)
  set(etherlab_include /usr/local/include)
  set(etherlab_lib /usr/local/lib)
elseif(BUILD_RPI3)
  set(etherlab_include /usr/include/etherlab)
  set(etherlab_lib /usr/lib)
elseif(BUILD_RPI4)
  set(etherlab_include /usr/include)
  set(etherlab_lib /usr/lib)
else()
  set(etherlab_include /usr/include)
  set(etherlab_lib /usr/lib)
endif()
set(ecat_node_include ${CMAKE_CURRENT_SOURCE_DIR}/include/)
set(node_name "ecat_node")

## Finding packages that'll be required for compilation.
## Don't forget to add packages if you use it in your code, otherwise you'll get build errors.

## Output executable name and requied cpp files for executable

if(NOT DEFINED BUILD_TEST)
  file(GLOB ecat_node_SRC
      "src/ec-server/v1/*.c"
  )

  file(GLOB ecat_req_SRC
      "src/ec-client/v1/request/*.c"
  )

  file(GLOB ecat_cli_SRC
      "src/ec-client/v1/cmd/*.c"
  )
else()
  file(GLOB ecat_node_SRC
    "agvethcat/test/*.c"
    "agvethcat/sock/test.c"
    "agvethcat/utils/*.c"
  )

  file(GLOB ecat_req_SRC
    "agvethcat/request/*.c"
    "agvethcat/utils/*.c"
  )

  file(GLOB ecat_cli_SRC
    "agvethcat/cmd/*.c"
  )

endif()
# Add include directories
include_directories(
  ${etherlab_include}
  ${ecat_node_include}
)


link_directories(
  ${etherlab_lib}
)

add_library(ecrequest
  SHARED
  ${ecat_req_SRC}
)


add_executable(ecctl ${ecat_cli_SRC})


add_executable(ecserver ${ecat_node_SRC})



## Specifying libraries by using definitions above.

target_link_libraries(
  ecctl
  PRIVATE
  ecrequest
)

if(NOT DEFINED BUILD_TEST)
  target_link_libraries(
    ecserver
    PRIVATE
    ethercat 
    Threads::Threads
    rt
  )
else()
  target_link_libraries(
    ecat_node
    PRIVATE
    Threads::Threads
    rt
  )

endif()


## Don't forget to add dependencies to your build file, 
## Use find_package(x) then add dependecy for x. 

install(TARGETS ecserver
  DESTINATION build/${PROJECT_NAME})
