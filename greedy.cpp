#include "timetable.h"

#include <stdexcept> // runtime_error

using namespace std;

// Usporeduje dva dogadaja kada treba prekinuti ciklus prethodnosti.
static bool hasHigherPriority(const Graph& graph, int first, int second) {
    if (graph.numberOfStudents[first] > graph.numberOfStudents[second]) {
        return true;
    }

    if (graph.numberOfStudents[first] < graph.numberOfStudents[second]) {
        return false;
    }

    if (graph.numberOfConflicts(first) > graph.numberOfConflicts(second)) {
        return true;
    }

    if (graph.numberOfConflicts(first) < graph.numberOfConflicts(second)) {
        return false;
    }

    return first < second;
}

// Priprema prazne strukture koje se koriste tijekom rasporedivanja.
GreedyTimetabler::GreedyTimetabler(const TimData& data, const Graph& graph)
    : data_(data),
      graph_(graph),
      schedule_(data.E),
      compatibleRooms_(data.E),
      roomEvent_(NUMBER_OF_TIMESLOTS, vector<int>(data.R, -1)),
      studentSchedule_(data.S, vector<int>(NUMBER_OF_TIMESLOTS, 0)),
      predecessors_(data.E),
      successors_(data.E),
      remainingPredecessors_(data.E, 0),
      processed_(data.E, false),
      neighbourTimeslotCount_(data.E, vector<int>(NUMBER_OF_TIMESLOTS, 0)),
      saturationDegree_(data.E, 0) {

    buildCompatibleRooms();
    buildPrecedenceGraph();
}

// Za svaki dogadaj sprema sve ucionice koje mu odgovaraju.
void GreedyTimetabler::buildCompatibleRooms() {
    for (int event = 0; event < data_.E; ++event) {
        for (int room = 0; room < data_.R; ++room) {
            if (roomSatisfiesEvent(room, event)) {
                compatibleRooms_[event].push_back(room);
            }
        }
    }
}

// Iz matrice prethodnosti stvara popise prethodnika i sljedbenika.
void GreedyTimetabler::buildPrecedenceGraph() {
    for (int first = 0; first < data_.E; ++first) {
        for (int second = 0; second < data_.E; ++second) {
            if (data_.precedence[first][second] == 1) {
                successors_[first].push_back(second);
                predecessors_[second].push_back(first);
            }
        }
    }

    for (int event = 0; event < data_.E; ++event) {
        remainingPredecessors_[event] =
            static_cast<int>(predecessors_[event].size());
    }
}

// Provjerava kapacitet i potrebne znacajke ucionice.
bool GreedyTimetabler::roomSatisfiesEvent(int room, int event) const {
    if (data_.roomSizes[room] < graph_.numberOfStudents[event]) {
        return false;
    }

    for (int feature = 0; feature < data_.F; ++feature) {
        bool eventNeedsFeature = data_.eventFeature[event][feature] == 1;
        bool roomHasFeature = data_.roomFeature[room][feature] == 1;

        if (eventNeedsFeature && !roomHasFeature) {
            return false;
        }
    }

    return true;
}

// Provjerava moze li se dogadaj staviti u zadani termin.
bool GreedyTimetabler::timeslotCanBeUsed(int event, int timeslot) const {
    if (timeslot < 0 || timeslot >= NUMBER_OF_TIMESLOTS) {
        return false;
    }

    // Dogadaj nije dostupan u tom terminu.
    if (data_.eventTimeslot[event][timeslot] == 0) {
        return false;
    }

    // Neki konfliktni dogadaj vec koristi termin.
    if (neighbourTimeslotCount_[event][timeslot] > 0) {
        return false;
    }

    // Mora postojati barem jedna slobodna odgovarajuca ucionica.
    bool freeRoomExists = false;

    for (int room : compatibleRooms_[event]) {
        if (roomEvent_[timeslot][room] == -1) {
            freeRoomExists = true;
            break;
        }
    }

    if (!freeRoomExists) {
        return false;
    }

    // Svaki rasporedeni prethodnik mora biti u ranijem terminu.
    for (int predecessor : predecessors_[event]) {
        if (schedule_[predecessor].isPlaced() &&
            schedule_[predecessor].timeslot >= timeslot) {
            return false;
        }
    }

    // Svaki rasporedeni sljedbenik mora biti u kasnijem terminu.
    for (int successor : successors_[event]) {
        if (schedule_[successor].isPlaced() &&
            schedule_[successor].timeslot <= timeslot) {
            return false;
        }
    }

    return true;
}

// Broji koliko termina trenutno odgovara dogadaju.
int GreedyTimetabler::countFeasibleTimeslots(int event) const {
    if (compatibleRooms_[event].empty()) {
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

// Odabire dogadaj koji je trenutno najteze rasporediti.
int GreedyTimetabler::chooseNextEvent(bool& brokePrecedenceCycle) const {
    brokePrecedenceCycle = false;

    int bestEvent = -1;
    int bestFeasibleTimeslots = 0;

    for (int event = 0; event < data_.E; ++event) {
        // Dogadaj mora biti neobraden i svi prethodnici moraju biti obradeni.
        if (processed_[event] || remainingPredecessors_[event] != 0) {
            continue;
        }

        int feasibleTimeslots = countFeasibleTimeslots(event);
        bool better = false;

        if (bestEvent == -1) {
            better = true;
        } else if (feasibleTimeslots < bestFeasibleTimeslots) {
            better = true;
        } else if (feasibleTimeslots == bestFeasibleTimeslots) {
            int students = graph_.numberOfStudents[event];
            int bestStudents = graph_.numberOfStudents[bestEvent];

            if (students > bestStudents) {
                better = true;
            } else if (students == bestStudents) {
                int saturation = saturationDegree_[event];
                int bestSaturation = saturationDegree_[bestEvent];

                if (saturation > bestSaturation) {
                    better = true;
                } else if (saturation == bestSaturation) {
                    int conflicts = graph_.numberOfConflicts(event);
                    int bestConflicts = graph_.numberOfConflicts(bestEvent);

                    if (conflicts > bestConflicts) {
                        better = true;
                    } else if (conflicts == bestConflicts) {
                        int available = data_.numberOfAvailableTimeslots(event);
                        int bestAvailable = data_.numberOfAvailableTimeslots(bestEvent);

                        if (available < bestAvailable) {
                            better = true;
                        } else if (available == bestAvailable && event < bestEvent) {
                            better = true;
                        }
                    }
                }
            }
        }

        if (better) {
            bestEvent = event;
            bestFeasibleTimeslots = feasibleTimeslots;
        }
    }

    if (bestEvent != -1) {
        return bestEvent;
    }

    // Ako nema dostupnog dogadaja, moguce je da postoji ciklus prethodnosti.
    brokePrecedenceCycle = true;

    for (int event = 0; event < data_.E; ++event) {
        if (processed_[event]) {
            continue;
        }

        if (bestEvent == -1) {
            bestEvent = event;
            continue;
        }

        if (remainingPredecessors_[event] < remainingPredecessors_[bestEvent]) {
            bestEvent = event;
        } else if (remainingPredecessors_[event] ==
                       remainingPredecessors_[bestEvent] &&
                   hasHigherPriority(graph_, event, bestEvent)) {
            bestEvent = event;
        }
    }

    return bestEvent;
}

// Racuna meki trosak rasporeda jednog studenta tijekom jednog dana.
int GreedyTimetabler::dailyPenalty(const vector<int>& daySchedule) const {
    int numberOfClasses = 0;

    for (int slot = 0; slot < SLOTS_PER_DAY; ++slot) {
        if (daySchedule[slot] == 1) {
            ++numberOfClasses;
        }
    }

    int penalty = 0;

    // Student ne bi trebao imati samo jedno predavanje u danu.
    if (numberOfClasses == 1) {
        ++penalty;
    }

    // Student ne bi trebao imati predavanje u zadnjem terminu dana.
    if (daySchedule[SLOTS_PER_DAY - 1] == 1) {
        ++penalty;
    }

    int consecutiveClasses = 0;

    for (int slot = 0; slot < SLOTS_PER_DAY; ++slot) {
        if (daySchedule[slot] == 1) {
            ++consecutiveClasses;
        } else {
            if (consecutiveClasses >= 3) {
                penalty += consecutiveClasses - 2;
            }

            consecutiveClasses = 0;
        }
    }

    // Ovaj dio obraduje niz koji zavrsava u zadnjem terminu dana.
    if (consecutiveClasses >= 3) {
        penalty += consecutiveClasses - 2;
    }

    return penalty;
}

// Racuna promjenu mekog troska ako se dogadaj doda u termin.
int GreedyTimetabler::softCostIncrease(int event, int timeslot) const {
    int day = timeslot / SLOTS_PER_DAY;
    int firstSlotOfDay = day * SLOTS_PER_DAY;
    int slotInsideDay = timeslot % SLOTS_PER_DAY;
    int totalIncrease = 0;

    for (int student : graph_.studentsOfEvent[event]) {
        vector<int> before(SLOTS_PER_DAY, 0);

        for (int slot = 0; slot < SLOTS_PER_DAY; ++slot) {
            before[slot] = studentSchedule_[student][firstSlotOfDay + slot];
        }

        vector<int> after = before;
        after[slotInsideDay] = 1;

        totalIncrease += dailyPenalty(after) - dailyPenalty(before);
    }

    return totalIncrease;
}

// Procjenjuje koliko bi odabrani termin blokirao jos neobradene susjede.
long long GreedyTimetabler::blockingCost(int event, int timeslot) const {
    long long cost = 0;

    for (int neighbour : graph_.conflictList[event]) {
        if (processed_[neighbour]) {
            continue;
        }

        bool neighbourCanUseTimeslot =
            data_.eventTimeslot[neighbour][timeslot] == 1;
        bool timeslotIsNotAlreadyBlocked =
            neighbourTimeslotCount_[neighbour][timeslot] == 0;

        if (neighbourCanUseTimeslot && timeslotIsNotAlreadyBlocked) {
            int numberOfStudents = graph_.numberOfStudents[neighbour];

            if (numberOfStudents < 1) {
                numberOfStudents = 1;
            }

            cost += numberOfStudents;
        }
    }

    return cost;
}

// Odabire najbolji termin i ucionicu za zadani dogadaj.
GreedyTimetabler::Placement GreedyTimetabler::choosePlacement(int event) const {
    Placement best;
    bool bestExists = false;

    for (int timeslot = 0; timeslot < NUMBER_OF_TIMESLOTS; ++timeslot) {
        if (!timeslotCanBeUsed(event, timeslot)) {
            continue;
        }

        int increase = softCostIncrease(event, timeslot);
        long long blocking = blockingCost(event, timeslot);

        for (int room : compatibleRooms_[event]) {
            if (roomEvent_[timeslot][room] != -1) {
                continue;
            }

            Placement candidate;
            candidate.timeslot = timeslot;
            candidate.room = room;
            candidate.softCostIncrease = increase;
            candidate.blockingCost = blocking;
            candidate.unusedSeats =
                data_.roomSizes[room] - graph_.numberOfStudents[event];

            bool better = false;

            if (!bestExists) {
                better = true;
            } else if (candidate.blockingCost < best.blockingCost) {
                better = true;
            } else if (candidate.blockingCost == best.blockingCost &&
                       candidate.softCostIncrease < best.softCostIncrease) {
                better = true;
            } else if (candidate.blockingCost == best.blockingCost &&
                       candidate.softCostIncrease == best.softCostIncrease &&
                       candidate.timeslot < best.timeslot) {
                better = true;
            } else if (candidate.blockingCost == best.blockingCost &&
                       candidate.softCostIncrease == best.softCostIncrease &&
                       candidate.timeslot == best.timeslot &&
                       candidate.unusedSeats < best.unusedSeats) {
                better = true;
            } else if (candidate.blockingCost == best.blockingCost &&
                       candidate.softCostIncrease == best.softCostIncrease &&
                       candidate.timeslot == best.timeslot &&
                       candidate.unusedSeats == best.unusedSeats &&
                       candidate.room < best.room) {
                better = true;
            }

            if (better) {
                best = candidate;
                bestExists = true;
            }
        }
    }

    return best;
}

// Oznacava dogadaj kao obraden i oslobada njegove sljedbenike.
void GreedyTimetabler::markProcessed(int event) {
    if (processed_[event]) {
        return;
    }

    processed_[event] = true;

    for (int successor : successors_[event]) {
        if (!processed_[successor] && remainingPredecessors_[successor] > 0) {
            --remainingPredecessors_[successor];
        }
    }
}

// Sprema dogadaj u raspored i azurira pomocne podatke.
void GreedyTimetabler::placeEvent(int event, const Placement& placement) {
    schedule_[event].timeslot = placement.timeslot;
    schedule_[event].room = placement.room;

    roomEvent_[placement.timeslot][placement.room] = event;

    for (int student : graph_.studentsOfEvent[event]) {
        studentSchedule_[student][placement.timeslot] = 1;
    }

    for (int neighbour : graph_.conflictList[event]) {
        bool firstConflictInThisTimeslot =
            neighbourTimeslotCount_[neighbour][placement.timeslot] == 0;

        if (firstConflictInThisTimeslot && !processed_[neighbour]) {
            ++saturationDegree_[neighbour];
        }

        ++neighbourTimeslotCount_[neighbour][placement.timeslot];
    }

    markProcessed(event);
}

// Glavna metoda pohlepnog algoritma.
Schedule GreedyTimetabler::solve() {
    int processedCount = 0;

    while (processedCount < data_.E) {
        bool brokePrecedenceCycle = false;
        int event = chooseNextEvent(brokePrecedenceCycle);

        if (event < 0) {
            throw runtime_error("Nije moguce odabrati sljedeci dogadaj.");
        }

        // Dogadaj koji prekida ciklus ostaje nerasporeden.
        if (brokePrecedenceCycle) {
            markProcessed(event);
            ++processedCount;
            continue;
        }

        Placement placement = choosePlacement(event);

        if (placement.exists()) {
            placeEvent(event, placement);
        } else {
            // Ako nema valjane pozicije, dogadaj ostaje nerasporeden.
            markProcessed(event);
        }

        ++processedCount;
    }

    return schedule_;
}