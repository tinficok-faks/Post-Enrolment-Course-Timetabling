#ifndef READER_H
#define READER_H

#include <vector>
#include <string>
struct EventData {

    int S = 0;
    std::vector<int> roomSizes; // kapacitet ucionice r
    std::vector<std::vector<int>> placedEvents;
    std::vector<int> unplacedEvents;
    std::vector<std::vector<int>> conflictList;
    std::vector<std::vector<int>> studentsOfEvent;
    std::vector<std::vector<int>> roomFeature; // roomFeature[r][f] = 1 ako ucionica 'r' posjeduje znacajku 'f'; ako ne onda 0 
    std::vector<std::vector<int>> eventFeature; // eventFeature[e][f] = 1 ako dogadaj 'e' zahtijeva znacajku 'f'; ako ne onda 0
    std::vector<std::vector<int>> eventTimeslot; // eventTimeslot[e][t] = 1 znaci termin je dostupan; ako nije onda 0
    std::vector<std::vector<int>> precedence; // precedence[first][second] opisuje odnos redoslijeda:
};

EventData readfiles(std::string datasetNumber);

#endif