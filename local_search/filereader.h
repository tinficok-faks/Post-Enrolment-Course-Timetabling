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

void loadInputSolution(const std::string& filename, EventData& data);

struct Move {
    int event = -1;
    int timeslot = -1;
    int room = -1;
    int cost = 0;
    bool exists() const {
        return event != -1;
    }
    int secondEvent = -1;
    int room2 = -1;
};


class SearchHelper {
public:
    std::vector<int>& roomSizes;
    std::vector<std::vector<int>>& placedEvents;
    std::vector<int>& unplacedEvents;

    const std::vector<std::vector<int>>& conflictList;
    const std::vector<std::vector<int>>& studentsOfEvent;
    const std::vector<std::vector<int>>& roomFeature;
    const std::vector<std::vector<int>>& eventFeature;
    const std::vector<std::vector<int>>& eventTimeslot;
    const std::vector<std::vector<int>>& precedence;

    int numberOfEvents;
    int numberOfTimeslots;
    int numberOfRooms;
    int S;

    std::vector<int> currentTimeslot;
    std::vector<int> currentRoom;

    // tabuUntil[event][timeslot]
    std::vector<std::vector<int>> tabuUntil;

    int currentIteration = 0;
    int bestCost;

    SearchHelper(
        std::vector<int>& roomSizes,
        std::vector<std::vector<int>>& placedEvents,
        std::vector<int>& unplacedEvents,
        const std::vector<std::vector<int>>& conflictList,
        const std::vector<std::vector<int>>& studentsOfEvent,
        const std::vector<std::vector<int>>& roomFeature,
        const std::vector<std::vector<int>>& eventFeature,
        const std::vector<std::vector<int>>& eventTimeslot,
        const std::vector<std::vector<int>>& precedence,
        int S
    );

    void initializePositions();

    bool roomCompatible(int event, int room);

};

#endif