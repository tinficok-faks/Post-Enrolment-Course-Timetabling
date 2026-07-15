#include "timetable.h"

#include <algorithm>
#include <stdexcept>

using namespace std;

// odreduje koji dogadaj treba zadrzati kada dva dogadaja stvaraju konflikt
static bool hasHigherPriority(const Graph& graph, int first, int second) {
    if (graph.numberOfStudents[first] != graph.numberOfStudents[second])
        return graph.numberOfStudents[first] > graph.numberOfStudents[second];

    if (graph.numberOfConflicts(first) != graph.numberOfConflicts(second))
        return graph.numberOfConflicts(first) > graph.numberOfConflicts(second);

    return first < second;
}

ValidationResult validateSchedule(const TimData& data,
                                  const Graph& graph,
                                  const Schedule& schedule,
                                  size_t maximumReportedErrors) {
    ValidationResult result;

    auto addError = [&](const string& message) {
        result.valid = false;
        if (result.errors.size() < maximumReportedErrors) {
            result.errors.push_back(message);
        }
    };

    if (static_cast<int>(schedule.size()) != data.E) {
        addError("Raspored nema točno jedan zapis za svaki događaj.");
        return result;
    }

    vector<vector<int>> roomEvent(
        NUMBER_OF_TIMESLOTS, vector<int>(data.R, -1));
    vector<vector<int>> studentEvent(
        data.S, vector<int>(NUMBER_OF_TIMESLOTS, -1));

    for (int event = 0; event < data.E; ++event) {
        const Assignment assignment = schedule[event];
        if (!assignment.isPlaced()) {
            if (assignment.timeslot != -1 || assignment.room != -1) {
                addError("Događaj " + to_string(event) +
                         " mora imati ili oba indeksa -1 ili oba valjana indeksa.");
            }
            continue;
        }

        if (assignment.timeslot < 0 || assignment.timeslot >= NUMBER_OF_TIMESLOTS) {
            addError("Događaj " + to_string(event) + " ima neispravan termin.");
            continue;
        }
        if (assignment.room < 0 || assignment.room >= data.R) {
            addError("Događaj " + to_string(event) + " ima neispravnu učionicu.");
            continue;
        }

        if (data.eventTimeslot[event][assignment.timeslot] == 0) {
            addError("Događaj " + to_string(event) +
                     " je stavljen u nedostupan termin " +
                     to_string(assignment.timeslot) + ".");
        }

        if (data.roomSizes[assignment.room] < graph.numberOfStudents[event]) {
            addError("Učionica " + to_string(assignment.room) +
                     " premala je za događaj " + to_string(event) + ".");
        }
        for (int feature = 0; feature < data.F; ++feature) {
            if (data.eventFeature[event][feature] == 1 &&
                data.roomFeature[assignment.room][feature] == 0) {
                addError("Učionica " + to_string(assignment.room) +
                         " nema značajku " + to_string(feature) +
                         " potrebnu događaju " + to_string(event) + ".");
            }
        }

        int& occupiedBy = roomEvent[assignment.timeslot][assignment.room];
        if (occupiedBy != -1) {
            addError("Događaji " + to_string(occupiedBy) + " i " +
                     to_string(event) + " koriste istu učionicu i termin.");
        } else {
            occupiedBy = event;
        }

        for (const int student : graph.studentsOfEvent[event]) {
            int& otherEvent = studentEvent[student][assignment.timeslot];
            if (otherEvent != -1) {
                addError("Student " + to_string(student) +
                         " istodobno sluša događaje " + to_string(otherEvent) +
                         " i " + to_string(event) + ".");
            } else {
                otherEvent = event;
            }
        }
    }

    for (int first = 0; first < data.E; ++first) {
        if (!schedule[first].isPlaced()) {
            continue;
        }
        for (int second = 0; second < data.E; ++second) {
            if (data.precedence[first][second] == 1 &&
                schedule[second].isPlaced() &&
                schedule[first].timeslot >= schedule[second].timeslot) {
                addError("Prethodnost nije zadovoljena: događaj " +
                         to_string(first) + " mora biti prije događaja " +
                         to_string(second) + ".");
            }
        }
    }

    return result;
}

void repairToValid(const TimData& data, const Graph& graph, Schedule& schedule) {
    if (static_cast<int>(schedule.size()) != data.E) {
        schedule.resize(data.E);
    }

    // uklanjanje pojedinacno neispravne dodjele
    for (int event = 0; event < data.E; ++event) {
        const Assignment assignment = schedule[event];
        bool individuallyValid = assignment.isPlaced();
        if (!assignment.isPlaced()) {
            schedule[event] = {-1, -1};
            continue;
        }
        if (assignment.timeslot < 0 || assignment.timeslot >= NUMBER_OF_TIMESLOTS ||
            assignment.room < 0 || assignment.room >= data.R) {
            individuallyValid = false;
        }
        if (individuallyValid && data.eventTimeslot[event][assignment.timeslot] == 0) {
            individuallyValid = false;
        }
        if (individuallyValid &&
            data.roomSizes[assignment.room] < graph.numberOfStudents[event]) {
            individuallyValid = false;
        }
        if (individuallyValid) {
            for (int feature = 0; feature < data.F; ++feature) {
                if (data.eventFeature[event][feature] == 1 &&
                    data.roomFeature[assignment.room][feature] == 0) {
                    individuallyValid = false;
                    break;
                }
            }
        }
        if (!individuallyValid) {
            schedule[event] = {-1, -1};
        }
    }

    // kod konflikata zadrzavamo događaj čije bi uklanjanje vise povecalo udaljenost do dopustivosti.
    vector<int> order(data.E);
    for (int event = 0; event < data.E; ++event) {
        order[event] = event;
    }
    sort(order.begin(), order.end(),
              [&](int first, int second) {
                  return hasHigherPriority(graph, first, second);
              });

    Schedule kept(data.E);
    vector<vector<int>> roomEvent(
        NUMBER_OF_TIMESLOTS, vector<int>(data.R, -1));
    vector<unsigned long long> studentMask(data.S, 0ULL);

    for (const int event : order) {
        const Assignment assignment = schedule[event];
        if (!assignment.isPlaced()) {
            continue;
        }

        bool canKeep = roomEvent[assignment.timeslot][assignment.room] == -1;
        for (const int student : graph.studentsOfEvent[event]) {
            if ((studentMask[student] & (1ULL << assignment.timeslot)) != 0ULL) {
                canKeep = false;
                break;
            }
        }
        if (!canKeep) {
            continue;
        }

        kept[event] = assignment;
        roomEvent[assignment.timeslot][assignment.room] = event;
        for (const int student : graph.studentsOfEvent[event]) {
            studentMask[student] |= (1ULL << assignment.timeslot);
        }
    }
    schedule = kept;

    // uklanjanje jednog dogadaja iz svake narusene relacije prethodnosti
    bool changed = true;
    while (changed) {
        changed = false;
        for (int first = 0; first < data.E && !changed; ++first) {
            if (!schedule[first].isPlaced()) {
                continue;
            }
            for (int second = 0; second < data.E; ++second) {
                if (data.precedence[first][second] != 1 ||
                    !schedule[second].isPlaced()) {
                    continue;
                }
                if (schedule[first].timeslot >= schedule[second].timeslot) {
                    const int removeEvent = hasHigherPriority(graph, first, second)
                                                ? second
                                                : first;
                    schedule[removeEvent] = {-1, -1};
                    changed = true;
                    break;
                }
            }
        }
    }
}
