#ifndef TIMETABLE_H
#define TIMETABLE_H

#include "graph.h"
#include "parser.h"

#include <ostream>
#include <string>
#include <vector>


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
using Schedule = std::vector<Assignment>;


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
    std::vector<std::string> errors;
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
    std::vector<std::vector<int>> compatibleRooms_;

   //zapis odgovarajucih i zauzetih ucionica (1 ili 0)
    std::vector<unsigned int> compatibleRoomMask_;
    std::vector<unsigned int> occupiedRoomMask_;

    // roomEvent_[timeslot][room] sadrzi indeks dogadaja ili -1 ako je ucionica slobodna
    std::vector<std::vector<int>> roomEvent_;

    // raspored svakog studenta (1 ili 0)
    std::vector<std::vector<int>> studentSchedule_;

    // podaci o redoslijedu dogadaja
    std::vector<std::vector<int>> predecessors_;
    std::vector<std::vector<int>> successors_;
    std::vector<int> remainingPredecessors_;

    // processed_[event] oznacava je li dogadaj vec obraden
    std::vector<bool> processed_;

    // broj konfliktnih susjeda u svakom terminu
    std::vector<std::vector<int>> neighbourTimeslotCount_;

    // dinamicki stupanj zasicenja svakog dogadaja
    std::vector<int> saturationDegree_;

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
        const std::vector<int>& daySchedule
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
    std::size_t maximumReportedErrors = 100
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
    std::ostream& output,
    const TimData& data,
    const Schedule& schedule,
    const std::string& instanceName
);

void writeReadableTimetableFile(
    const std::string& filename,
    const TimData& data,
    const Schedule& schedule,
    const std::string& instanceName
);
#endif
