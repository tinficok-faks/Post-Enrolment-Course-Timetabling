#include "graph.h"
#include <stdexcept> // invalid_argument i out_of_range


// numberOfEvents je broj dogadaja u problemu
// po njemu se stvaraju matrice i vektori odgovarajucih dimenzija
Graph::Graph(int numberOfEvents):
        eventsConflict(numberOfEvents, std::vector<int>(numberOfEvents, 0)),
        conflictList(numberOfEvents),
        studentsOfEvent(numberOfEvents){
            
    if (numberOfEvents <= 0) {
        throw std::invalid_argument("Broj dogadaja mora biti pozitivan");
    }
}

//metoda prolazi svakog studenta i biljezi koji su dogadaji u konfliktu i
//koliko studenata slusa svaki dogadaja
//te podatke sprema u za to predvidene vektore
void Graph::fillVector(const std::vector<std::vector<int>>& studentEvent) {
    if (studentEvent.empty()) {
        return;
    }

    const int number_Of_Events = eventsConflict.size();
    for (int s = 0; s < (int)(studentEvent.size()); ++s) {

        std::vector<int> attendedEvents;
        for (int e = 0; e < number_Of_Events; ++e) {
            if (studentEvent[s][e] == 1) {
                attendedEvents.push_back(e);
                studentsOfEvent[e].push_back(s);
            }
        }

        for (int i = 0; i < (int)(attendedEvents.size()); ++i) {
            for (int j = i + 1; j < (int)(attendedEvents.size()); ++j) {
                const int first = attendedEvents[i];
                const int second = attendedEvents[j];
                if (!eventsConflict[first][second]) {
                    eventsConflict[first][second] = 1;
                    eventsConflict[second][first] = 1;
                    conflictList[first].push_back(second);
                    conflictList[second].push_back(first);
                }
            }
        }
    }
}

//metoda vraca koliko je dogadaja u konfliktu s eventom
int Graph::numberOfConflicts(int event) const {
    if (event < 0 || event >= (int)(conflictList.size())) {
        throw std::out_of_range("Neispravan indeks dogadaja");
    }
    return conflictList[event].size();
}

//metoda vraca jesu li firstEvent i secondEvent u konfliktu
bool Graph::conflict(int firstEvent, int secondEvent) const {
    if (firstEvent < 0 || secondEvent < 0 ||
        firstEvent >= (int)(eventsConflict.size()) ||
        secondEvent >= (int)(eventsConflict.size())) {
        throw std::out_of_range("Neispravan indeks dogadaja");
    }
    return eventsConflict[firstEvent][secondEvent] != 0;
}
