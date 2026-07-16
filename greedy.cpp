#include "timetable.h"

#include <algorithm> // max i sort
#include <bit> // popcount za C++20
#include <limits> // numeric_limits
#include <stdexcept> // runtime_error
#include <tuple> // usporedba vise kriterija uz tuple

using namespace std;

// Pomocna funkcija za brojanje jedinica u binarnom zapisu broja
static int countBits(unsigned int value) {
#if __cplusplus >= 202002L // ovo je ako compileamo s std=c++20
    return popcount(value);
#else
    return __builtin_popcount(value); // a ovo je ako po std=c++17 i mozda starije
#endif
}

// usporedjuje prioritet 2 dogadaja
// kriterij prioriteta: 1. dogadaj s vise studenata
//                      2. s vise konflikata
//                      3. ako su 1. jednak 2. onda onaj s manjim indeksom
static bool hasHigherPriority(const Graph& graph, int first, int second) {
    if (graph.numberOfStudents[first] != graph.numberOfStudents[second])
        return graph.numberOfStudents[first] > graph.numberOfStudents[second];
    if (graph.numberOfConflicts(first) != graph.numberOfConflicts(second))
        return graph.numberOfConflicts(first) > graph.numberOfConflicts(second);
    return first < second;
}

// priprema potrebnih pomocnih struktura za kasnije
GreedyTimetabler::GreedyTimetabler(const TimData& data, const Graph& graph) :
        data_(data), // data_ = data
        graph_(graph), // graph_ = graph
        schedule_(data.E),
        compatibleRoomMask_(data.E, 0U),
        compatibleRooms_(data.E),
        occupiedRoomMask_(NUMBER_OF_TIMESLOTS, 0U),
        roomEvent_(NUMBER_OF_TIMESLOTS, vector<int>(data.R, -1)),
        studentScheduleMask_(data.S, 0ULL),
        predecessors_(data.E),
        successors_(data.E),
        remainingPredecessors_(data.E, 0),
        processed_(data.E, 0),
        neighbourTimeslotCount_(data.E, vector<int>(NUMBER_OF_TIMESLOTS, 0)),
        saturationDegree_(data.E, 0) {
    
    buildCompatibleRooms();
    buildPrecedenceGraph();
}

void GreedyTimetabler::buildCompatibleRooms() {
    for (int event = 0; event < data_.E; ++event) {
        for (int room = 0; room < data_.R; ++room) {
            if (roomSatisfiesEvent(room, event)) {
                compatibleRooms_[event].push_back(room);
                compatibleRoomMask_[event] |= (1U << room);
            }
        }

        sort(compatibleRooms_[event].begin(), compatibleRooms_[event].end(),
                  [&](int first, int second) {
                      if (data_.roomSizes[first] != data_.roomSizes[second]) {
                          return data_.roomSizes[first] < data_.roomSizes[second];
                      }
                      return first < second;
                  });
    }
}

void GreedyTimetabler::buildPrecedenceGraph() {
    for (int first = 0; first < data_.E; ++first) {
        for (int second = 0; second < data_.E; ++second) {
            // Vrijednost 1 znaci: first mora biti prije second.
            if (data_.precedence[first][second] == 1) {
                successors_[first].push_back(second);
                predecessors_[second].push_back(first);
            }
        }
    }

    for (int event = 0; event < data_.E; ++event) {
        remainingPredecessors_[event] = static_cast<int>(predecessors_[event].size());
    }
}

bool GreedyTimetabler::roomSatisfiesEvent(int room, int event) const {
    if (data_.roomSizes[room] < graph_.numberOfStudents[event]) {
        return false;
    }

    for (int feature = 0; feature < data_.F; ++feature) {
        if (data_.eventFeature[event][feature] == 1 &&
            data_.roomFeature[room][feature] == 0) {
            return false;
        }
    }
    return true;
}

bool GreedyTimetabler::timeslotCanBeUsed(int event, int timeslot) const {
    if (timeslot < 0 || timeslot >= NUMBER_OF_TIMESLOTS) {
        return false;
    }
    if (data_.eventTimeslot[event][timeslot] == 0) {
        return false;
    }
    if (neighbourTimeslotCount_[event][timeslot] != 0) {
        return false;
    }

    const unsigned int freeCompatibleRooms =
        compatibleRoomMask_[event] & ~occupiedRoomMask_[timeslot];
    if (freeCompatibleRooms == 0U) {
        return false;
    }

    for (const int predecessor : predecessors_[event]) {
        if (schedule_[predecessor].isPlaced() &&
            schedule_[predecessor].timeslot >= timeslot) {
            return false;
        }
    }
    for (const int successor : successors_[event]) {
        if (schedule_[successor].isPlaced() &&
            timeslot >= schedule_[successor].timeslot) {
            return false;
        }
    }

    return true;
}

int GreedyTimetabler::countFeasibleTimeslots(int event) const {
    if (compatibleRoomMask_[event] == 0U) {
        return 0;
    }

    int count = 0;
    for (int timeslot = 0; timeslot < NUMBER_OF_TIMESLOTS; ++timeslot) {
        if (timeslotCanBeUsed(event, timeslot)) {
            ++count;
        }
    }
    return count;
}

int GreedyTimetabler::chooseNextEvent(bool& brokePrecedenceCycle) const {
    brokePrecedenceCycle = false;
    int bestEvent = -1;
    int bestFeasibleTimeslots = numeric_limits<int>::max();

    auto betterCandidate = [&](int event, int feasibleTimeslots) {
        if (bestEvent == -1) {
            return true;
        }

        // Pohlepni redoslijed:
        // 1. najmanje trenutno mogucih termina (fail-first / least saturation),
        // 2. vise studenata (veci moguci doprinos udaljenosti do dopustivosti),
        // 3. veci dinamicki stupanj zasicenja,
        // 4. vise konflikata,
        // 5. manje staticki dostupnih termina.
        return tuple<int, int, int, int, int, int>(
                   feasibleTimeslots,
                   -graph_.numberOfStudents[event],
                   -saturationDegree_[event],
                   -graph_.numberOfConflicts(event),
                   data_.numberOfAvailableTimeslots(event),
                   event) <
               tuple<int, int, int, int, int, int>(
                   bestFeasibleTimeslots,
                   -graph_.numberOfStudents[bestEvent],
                   -saturationDegree_[bestEvent],
                   -graph_.numberOfConflicts(bestEvent),
                   data_.numberOfAvailableTimeslots(bestEvent),
                   bestEvent);
    };

    for (int event = 0; event < data_.E; ++event) {
        if (processed_[event] || remainingPredecessors_[event] != 0) {
            continue;
        }
        const int feasibleTimeslots = countFeasibleTimeslots(event);
        if (betterCandidate(event, feasibleTimeslots)) {
            bestEvent = event;
            bestFeasibleTimeslots = feasibleTimeslots;
        }
    }

    if (bestEvent != -1) {
        return bestEvent;
    }

    // Zastita od ciklusa u matrici prethodnosti. U sluzbenim instancama se
    // ciklus ne ocekuje. Jedan dogadaj ostavljamo nerasporeden kako bismo
    // prekinuli ciklus i zadrzali valjan raspored.
    brokePrecedenceCycle = true;
    for (int event = 0; event < data_.E; ++event) {
        if (!processed_[event] &&
            (bestEvent == -1 ||
             remainingPredecessors_[event] < remainingPredecessors_[bestEvent] ||
             (remainingPredecessors_[event] == remainingPredecessors_[bestEvent] &&
              hasHigherPriority(graph_, event, bestEvent)))) {
            bestEvent = event;
        }
    }
    return bestEvent;
}

int GreedyTimetabler::dailyPenalty(unsigned int nineSlotMask) const {
    const int classes = countBits(nineSlotMask);
    int penalty = (classes == 1) ? 1 : 0;

    if ((nineSlotMask & (1U << (SLOTS_PER_DAY - 1))) != 0U) {
        ++penalty;
    }

    int runLength = 0;
    for (int slot = 0; slot < SLOTS_PER_DAY; ++slot) {
        if ((nineSlotMask & (1U << slot)) != 0U) {
            ++runLength;
        } else {
            if (runLength >= 3) {
                penalty += runLength - 2;
            }
            runLength = 0;
        }
    }
    if (runLength >= 3) {
        penalty += runLength - 2;
    }

    return penalty;
}

int GreedyTimetabler::softCostIncrease(int event, int timeslot) const {
    const int day = timeslot / SLOTS_PER_DAY;
    const int offset = day * SLOTS_PER_DAY;
    const unsigned int dayMask = (1U << SLOTS_PER_DAY) - 1U;
    int delta = 0;

    for (const int student : graph_.studentsOfEvent[event]) {
        const unsigned int before = static_cast<unsigned int>(
            (studentScheduleMask_[student] >> offset) & dayMask);
        const unsigned int after = before | (1U << (timeslot % SLOTS_PER_DAY));
        delta += dailyPenalty(after) - dailyPenalty(before);
    }
    return delta;
}

long long GreedyTimetabler::blockingCost(int event, int timeslot) const {
    long long cost = 0;
    for (const int neighbour : graph_.conflictList[event]) {
        if (processed_[neighbour]) {
            continue;
        }
        if (data_.eventTimeslot[neighbour][timeslot] == 1 &&
            neighbourTimeslotCount_[neighbour][timeslot] == 0) {
            // Blokiranje dogadaja s mnogo studenata skuplje je jer bi njegovo
            // nerasporedivanje vise povecalo udaljenost do dopustivosti.
            cost += max(1, graph_.numberOfStudents[neighbour]);
        }
    }
    return cost;
}

GreedyTimetabler::Placement GreedyTimetabler::choosePlacement(int event) const {
    Placement best;
    bool hasBest = false;

    for (int timeslot = 0; timeslot < NUMBER_OF_TIMESLOTS; ++timeslot) {
        if (!timeslotCanBeUsed(event, timeslot)) {
            continue;
        }

        const int increase = softCostIncrease(event, timeslot);
        const long long blocking = blockingCost(event, timeslot);

        for (const int room : compatibleRooms_[event]) {
            if (roomEvent_[timeslot][room] != -1) {
                continue;
            }

            Placement candidate;
            candidate.timeslot = timeslot;
            candidate.room = room;
            candidate.softCostIncrease = increase;
            candidate.blockingCost = blocking;
            candidate.unusedSeats = data_.roomSizes[room] - graph_.numberOfStudents[event];

            const auto candidateKey = tuple<long long, int, int, int, int>(
                candidate.blockingCost,
                candidate.softCostIncrease,
                candidate.timeslot,
                candidate.unusedSeats,
                candidate.room);
            const auto bestKey = tuple<long long, int, int, int, int>(
                best.blockingCost,
                best.softCostIncrease,
                best.timeslot,
                best.unusedSeats,
                best.room);

            if (!hasBest || candidateKey < bestKey) {
                best = candidate;
                hasBest = true;
            }
        }
    }

    return best;
}

void GreedyTimetabler::markProcessed(int event) {
    if (processed_[event]) {
        return;
    }
    processed_[event] = 1;
    for (const int successor : successors_[event]) {
        if (!processed_[successor] && remainingPredecessors_[successor] > 0) {
            --remainingPredecessors_[successor];
        }
    }
}

void GreedyTimetabler::placeEvent(int event, const Placement& placement) {
    schedule_[event] = {placement.timeslot, placement.room};
    roomEvent_[placement.timeslot][placement.room] = event;
    occupiedRoomMask_[placement.timeslot] |= (1U << placement.room);

    for (const int student : graph_.studentsOfEvent[event]) {
        studentScheduleMask_[student] |= (1ULL << placement.timeslot);
    }

    for (const int neighbour : graph_.conflictList[event]) {
        if (neighbourTimeslotCount_[neighbour][placement.timeslot] == 0 &&
            !processed_[neighbour]) {
            ++saturationDegree_[neighbour];
        }
        ++neighbourTimeslotCount_[neighbour][placement.timeslot];
    }

    markProcessed(event);
}

Schedule GreedyTimetabler::solve() {
    int processedCount = 0;
    while (processedCount < data_.E) {
        bool brokePrecedenceCycle = false;
        const int event = chooseNextEvent(brokePrecedenceCycle);
        if (event < 0) {
            throw runtime_error("Nije moguce odabrati sljedeci dogadaj.");
        }

        if (brokePrecedenceCycle) {
            markProcessed(event);
            ++processedCount;
            continue;
        }

        const Placement placement = choosePlacement(event);
        if (placement.exists()) {
            placeEvent(event, placement);
        } else {
            markProcessed(event);
        }
        ++processedCount;
    }

    return schedule_;
}
