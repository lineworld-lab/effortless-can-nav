
#pragma once
#include <vector>
#include <chrono>
#include <memory>
#include <functional>
#include <string>
#include <cstdint>
#include <ctime>
#include <ratio>
#include <fstream>
#include <string>
#include <iostream>

#define NUMBER_OF_SAMPLES 1E6
/**
 *  \class   Timing
 *  \brief   Contains Timing measurement related functions.
 */
class Timing{
    public:
      std::chrono::high_resolution_clock::time_point timer_start_;
      std::chrono::high_resolution_clock::time_point last_start_time_;
      std::chrono::duration<long,std::micro> time_span_;
      std::vector<long> timing_info_ = std::vector<long>(NUMBER_OF_SAMPLES);
      uint32_t counter_ = 0;
  /**
   * @brief Gets the current time and assings to timer_start_ member.
   */
  void GetTime();
  /**
   * @brief Measures time difference from last call to function GetTime() 
   * and writes is to time_span member
   * 
   */
  void MeasureTimeDifference();
  /**
   * @brief Outputs timing information to loop_timing_info.txt file.
   * 
   */
  void OutInfoToFile();
};