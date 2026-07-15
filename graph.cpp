#include "graph.h"
#include<stdexcept> // invalid_argument i out_of_range


// numberOfEvents je broj dogadaja u problemu
// po njemu se stvaraju matrice i vektori odgovarajucih dimenzija
Graph::Graph(int numberOfEvents):
        eventsConflict(numberOfEvents, vector<unsigned char>(numberOfEvents, 0)),
        conflictList(numberOfEvents),
        studentsOfEvent(numberOfEvents),
        numberOfStudents(numberOfEvents, 0) {
            
    if (numberOfEvents <= 0) {
        throw invalid_argument("Broj dogadaja mora biti pozitivan");
    }
}

void Graph::fillVector(const vector<vector<int>>& studentEvent) {
    if (studentEvent.empty()) {
        return;
    }

    const int numberOfEvents = static_cast<int>(eventsConflict.size());
    for (int s = 0; s < static_cast<int>(studentEvent.size()); ++s) {
        if (static_cast<int>(studentEvent[s].size()) != numberOfEvents) {
            throw invalid_argument("Matrica student-dogadaj ima pogresne dimenzije");
        }

        vector<int> attendedEvents;
        for (int e = 0; e < numberOfEvents; ++e) {
            if (studentEvent[s][e] == 1) {
                attendedEvents.push_back(e);
                studentsOfEvent[e].push_back(s);
                ++numberOfStudents[e];
            }
        }

        for (int i = 0; i < static_cast<int>(attendedEvents.size()); ++i) {
            for (int j = i + 1; j < static_cast<int>(attendedEvents.size()); ++j) {
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

int Graph::numberOfConflicts(int event) const {
    if (event < 0 || event >= static_cast<int>(conflictList.size())) {
        throw out_of_range("Neispravan indeks dogadaja");
    }
    return static_cast<int>(conflictList[event].size());
}

bool Graph::conflict(int firstEvent, int secondEvent) const {
    if (firstEvent < 0 || secondEvent < 0 ||
        firstEvent >= static_cast<int>(eventsConflict.size()) ||
        secondEvent >= static_cast<int>(eventsConflict.size())) {
        throw out_of_range("Neispravan indeks dogadaja");
    }
    return eventsConflict[firstEvent][secondEvent] != 0;
}
