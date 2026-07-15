#ifndef TIMETABLE_H
#define TIMETABLE_H

#include "graph.h"
#include "parser.h"

#include <ostream>
#include <string>
#include <vector>

using namespace std;

// termin i ucionica dodijeljeni jednom dogadaju
// -1 znaci da dogadaj nije rasporeden
struct Assignment {
    int timeslot = -1;
    int room = -1;

    bool isPlaced() const {
        return timeslot != -1 && room != -1;
    }
};

// schedule[event] daje termin i ucionicu tog dogadaja
using Schedule = vector<Assignment>;


// Rezultat evaluacije rasporeda.
struct Evaluation {
    int distanceToFeasibility = 0;
    int softCost = 0;
    int unplacedEvents = 0;

    int totalCost() const {
        return distanceToFeasibility + softCost;
    }
};


// rezultat provjere valjanosti rasporeda.
struct ValidationResult {
    bool valid = true;
    vector<string> errors;
};


class GreedyTimetabler {
public:
    // konstruktor prima podatke instance i graf konflikata
    GreedyTimetabler(
        const TimData& data,
        const Graph& graph
    );

    // stvara raspored pomocu pohlepnog algoritma
    Schedule solve();

private:
     // moguca pozicija jednog dogadaja
    struct Placement {
        int timeslot = -1;
        int room = -1;

        int softCostIncrease = 0;
        long long blockingCost = 0;
        int unusedSeats = 0;

        bool exists() const {
            return timeslot != -1 && room != -1;
        }
    };

    // osnovni podaci problema
    const TimData& data_;
    const Graph& graph_;

    // trenutni raspored
    Schedule schedule_;

    // compatibleRooms_[event] sadrzi ucionice koje odgovaraju dogadaju
    vector<vector<int>> compatibleRooms_;

   // bitovni zapis odgovarajucih i zauzetih ucionica
    vector<unsigned int> compatibleRoomMask_;
    vector<unsigned int> occupiedRoomMask_;

    // roomEvent_[timeslot][room] sadrzi indeks dogadaja ili -1 ako je ucionica slobodna
    vector<vector<int>> roomEvent_;

    // Raspored svakog studenta spremljen kao bitovni zapis (1/0)
    vector<unsigned long long> studentScheduleMask_;

    // Podaci o prethodnosti dogadaja
    vector<vector<int>> predecessors_;
    vector<vector<int>> successors_;
    vector<int> remainingPredecessors_;

    // processed_[event] oznacava je li dogadaj vec obraden
    vector<bool> processed_;

    // broj konfliktnih susjeda u svakom terminu
    vector<vector<int>> neighbourTimeslotCount_;

    // dinamicki stupanj zasicenja svakog dogadaja
    vector<int> saturationDegree_;

    // priprema podataka
    void buildCompatibleRooms();
    void buildPrecedenceGraph();

    // provjere ucionice i termina
    bool roomSatisfiesEvent(
        int room,
        int event
    ) const;

    bool timeslotCanBeUsed(
        int event,
        int timeslot
    ) const;

    int countFeasibleTimeslots(
        int event
    ) const;

    // odabir dogadaja i njegove pozicije
    int chooseNextEvent(
        bool& precedenceCycle
    ) const;

    Placement choosePlacement(
        int event
    ) const;

    // racunanje troskova
    int dailyPenalty(
        unsigned int dayMask
    ) const;

    int softCostIncrease(
        int event,
        int timeslot
    ) const;

    long long blockingCost(
        int event,
        int timeslot
    ) const;

    // azuriranje rasporeda
    void markProcessed(
        int event
    );

    void placeEvent(
        int event,
        const Placement& placement
    );
};


// provjerava krsi li raspored neki cvrsti uvjet
ValidationResult validateSchedule(
    const TimData& data,
    const Graph& graph,
    const Schedule& schedule,
    size_t maximumReportedErrors = 100
);


// Uklanja problematicke dogadaje dok raspored ne postane valjan
void repairToValid(
    const TimData& data,
    const Graph& graph,
    Schedule& schedule
);


// racuna udaljenost do dopustivosti i meki trosak
Evaluation evaluateSchedule(
    const TimData& data,
    const Graph& graph,
    const Schedule& schedule
);


// ispisuje citljiv raspored u terminal
void writeReadableTimetable(
    ostream& output,
    const TimData& data,
    const Schedule& schedule,
    const string& instanceName
);

#endif
