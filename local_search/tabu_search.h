#ifndef TABU_SEARCH_H
#define TABU_SEARCH_H

#include <vector>
#include <random>


struct Move {
    int event = -1;
    int timeslot = -1;
    int room = -1;
    std::vector<int> ejectedEvents;
    int cost = 0;
    bool exists() const {
        return event != -1;
    }
    int secondEvent = -1;
    int room2 = -1;
};


class TabuSearch {
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

    std::mt19937 rng;

    TabuSearch(
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

    bool solve(int maxIterations);

    void initializePositions();

    bool roomCompatible(int event, int room);

private:

    std::vector<int> collectConflicts(int event, int timeslot);

    Move findBestMove();

    void applyMove(const Move& move);

    bool contains(const std::vector<int>& v, int value);

};

#endif