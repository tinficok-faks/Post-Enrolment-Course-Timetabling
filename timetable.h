#ifndef TIMETABLE_H
#define TIMETABLE_H

#include "graph.h"
#include "parser.h"

#include <iosfwd>
#include <string>
#include <vector>

using namespace std;

struct Assignment {
    int timeslot = -1;
    int room = -1;

    bool isPlaced() const {
        return timeslot >= 0 && room >= 0;
    }
};

using Schedule = vector<Assignment>;

struct Evaluation {
    int distanceToFeasibility = 0;
    int softCost = 0;
    int unplacedEvents = 0;

    int totalCost() const {
        return distanceToFeasibility + softCost;
    }
};

struct ValidationResult {
    bool valid = true;
    vector<string> errors;
};

class GreedyTimetabler {
public:
    GreedyTimetabler(const TimData& data, const Graph& graph);

    Schedule solve();

private:
    struct Placement {
        int timeslot = -1;
        int room = -1;
        int softDelta = 0;
        long long futureBlocking = 0;
        int roomWaste = 0;

        bool exists() const {
            return timeslot >= 0 && room >= 0;
        }
    };

    const TimData& data_;
    const Graph& graph_;

    Schedule schedule_;
    vector<unsigned int> compatibleRoomMask_;
    vector<vector<int>> compatibleRooms_;
    vector<unsigned int> occupiedRoomMask_;
    vector<vector<int>> roomEvent_;
    vector<unsigned long long> studentScheduleMask_;

    vector<vector<int>> predecessors_;
    vector<vector<int>> successors_;
    vector<int> remainingPredecessors_;
    vector<unsigned char> processed_;

    vector<vector<unsigned short>> neighbourTimeslotCount_;
    vector<int> saturationDegree_;

    void buildCompatibleRooms();
    void buildPrecedenceGraph();

    bool roomSatisfiesEvent(int room, int event) const;
    int countCurrentlyFeasibleTimeslots(int event) const;
    bool timeslotCanBeUsed(int event, int timeslot) const;
    int chooseNextEvent(bool& brokePrecedenceCycle) const;
    Placement choosePlacement(int event) const;

    int dailyPenalty(unsigned int nineSlotMask) const;
    int incrementalSoftCost(int event, int timeslot) const;
    long long futureBlockingCost(int event, int timeslot) const;

    void markProcessed(int event);
    void place(int event, const Placement& placement);
};

ValidationResult validateSchedule(const TimData& data,
                                  const Graph& graph,
                                  const Schedule& schedule,
                                  size_t maximumReportedErrors = 100);

void repairToValid(const TimData& data, const Graph& graph, Schedule& schedule);
Evaluation evaluateSchedule(const TimData& data, const Graph& graph, const Schedule& schedule);

void writeSolutionFile(const string& filename, const Schedule& schedule);
void writeReadableTimetable(ostream& output,
                            const TimData& data,
                            const Schedule& schedule,
                            const string& instanceName);
void writeReadableTimetableFile(const string& filename,
                                const TimData& data,
                                const Schedule& schedule,
                                const string& instanceName);

#endif
