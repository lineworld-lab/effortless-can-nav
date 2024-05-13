#include "agvethcat/utils.h"




std::string GetStringTimeNow(){


    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    std::string ts = oss.str();

    return ts;

}

int GetTimeDiffMs(std::chrono::time_point<std::chrono::high_resolution_clock> t_start){

  auto t_now = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double> d_double;

  d_double = t_now - t_start;

  std::chrono::milliseconds sec = std::chrono::duration_cast<std::chrono::milliseconds>(d_double);

  return sec.count();

}

int WriteMoveStatusByAxis(int axis, std::string m_stat){

    std::string axis_str = std::to_string(axis);

    std::ofstream writer("/EC/move_" + axis_str);

    writer << m_stat;

    writer.close();

    return 0;

}


int ReadMoveStatusByAxis(int axis, std::string* m_stat){

    std::string axis_str = std::to_string(axis);

    std::ifstream reader("/EC/move_" + axis_str);

    reader >> *m_stat;

    reader.close();

    return 0;
}

int WriteHomingStatusByAxis(int axis, std::string h_stat){

    std::string axis_str = std::to_string(axis);

    std::ofstream writer("/EC/home_" + axis_str);

    writer << h_stat;

    writer.close();

    return 0;

}


int ReadHomingStatusByAxis(int axis, std::string* h_stat){

    std::string axis_str = std::to_string(axis);

    std::ifstream reader("/EC/home_" + axis_str);

    reader >> *h_stat;

    reader.close();

    return 0;
}
