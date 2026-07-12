#ifndef PARSER_H

#define PARSER_H

#include <vector>
#include <string>

struct TimData {
    int E, R, F, S;

    std::vector<int> roomSizes;
    std::vector<std::vector<int>> studentEvent; 
    std::vector<std::vector<int>> roomFeature;  
    std::vector<std::vector<int>> eventFeature; 
    std::vector<std::vector<int>> eventTimeslot;
    std::vector<std::vector<int>> precedence;   
};


TimData loadTIM(const std::string &filename);
#endif