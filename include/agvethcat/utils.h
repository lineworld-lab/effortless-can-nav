#ifndef _AGVETHCAT_H_UTILS_
#define _AGVETHCAT_H_UTILS_


#include "agvethcat.h"



std::string GetStringTimeNow();

int GetTimeDiffMs(std::chrono::time_point<std::chrono::high_resolution_clock>);

int WriteMoveStatusByAxis(int axis, std::string m_stat);

int ReadMoveStatusByAxis(int axis, std::string* m_stat);

int WriteHomingStatusByAxis(int axis, std::string h_stat);

int ReadHomingStatusByAxis(int axis, std::string* h_stat);



template <typename T> void fLog(T msg){

    std::ofstream outfile;

    outfile.open("/EC/log", std::ios_base::app);

    std::string now_str = GetStringTimeNow();

    now_str = "[ " + now_str + " ] ";

    outfile << now_str;

    outfile << msg;

    outfile << "\n";

    outfile.close();


}



template <typename T> void fClientLog(T msg){

    std::ofstream outfile;

    outfile.open("/EC/log_client", std::ios_base::app);

    std::string now_str = GetStringTimeNow();

    now_str = "[ " + now_str + " ] ";

    outfile << now_str;

    outfile << msg;

    outfile << "\n";

    outfile.close();


}








#endif