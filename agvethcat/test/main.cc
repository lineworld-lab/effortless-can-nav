#include "agvethcat/sock.h"




int main(int argc, char **argv){
 

  int server_stat = 0 ;


  server_stat = ListenAndServe_Test(SOCK_PORT);


  if(server_stat < 0 ){
    fLog<std::string>("server failure");

    exit(EXIT_FAILURE);
  }


  return 0;
}
