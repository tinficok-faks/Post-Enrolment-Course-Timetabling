#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>

constexpr int NUMBER_OF_DAYS = 5;
constexpr int SLOTS_PER_DAY = 9;
constexpr int NUMBER_OF_TIMESLOTS = NUMBER_OF_DAYS * SLOTS_PER_DAY;

struct TimData {
    int E = 0; // broj događaja
    int R = 0; // broj učionica
    int F = 0; // broj značajki
    int S = 0; // broj studenata

    std::vector<int> roomSizes;
    std::vector<std::vector<int>> studentEvent;
    std::vector<std::vector<int>> roomFeature;
    std::vector<std::vector<int>> eventFeature;
    std::vector<std::vector<int>> eventTimeslot;
    std::vector<std::vector<int>> precedence;

    int numberOfAvailableTimeslots(int event) const;
};

TimData loadTIM(const std::string& filename);

#endif


// #ifndef PARSER_H

// #define PARSER_H

// #include <vector>
// #include <string>

// struct TimData {
//     int E, R, F, S;

//     std::vector<int> roomSizes;
//     std::vector<std::vector<int>> studentEvent;
//     std::vector<std::vector<int>> roomFeature;
//     std::vector<std::vector<int>> eventFeature;
//     std::vector<std::vector<int>> eventTimeslot;
//     std::vector<std::vector<int>> precedence;

//     int numberof_timeslots(int i);
// };


// TimData loadTIM(const std::string &filename);
// #endif