#ifndef OUTPUT_H
#define OUTPUT_H

#include <string>
#include <vector>

void writeSolution(
    const std::string& fileName,
    int numberOfEvents,
    const std::vector<std::vector<int>>& placedEvents,
    int number
);
#endif