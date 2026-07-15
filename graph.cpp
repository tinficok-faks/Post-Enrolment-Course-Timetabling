#include "graph.h"
#include<stdexcept> // invalid_argument i out_of_range


Graph::Graph(int numberOfEvents):
        eventsConflict(numberOfEvents, vector<unsigned char>(numberOfEvents, 0)),
        conflictList(numberOfEvents),
        studentsOfEvent(numberOfEvents),
        numberOfStudents(numberOfEvents, 0) {
            
    if (numberOfEvents <= 0) {
        throw invalid_argument("Broj događaja mora biti pozitivan.");
    }
}

void Graph::fillVector(const vector<vector<int>>& studentEvent) {
    if (studentEvent.empty()) {
        return;
    }

    const int numberOfEvents = static_cast<int>(eventsConflict.size());
    for (int s = 0; s < static_cast<int>(studentEvent.size()); ++s) {
        if (static_cast<int>(studentEvent[s].size()) != numberOfEvents) {
            throw invalid_argument("Matrica student-događaj ima pogrešne dimenzije.");
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
        throw out_of_range("Neispravan indeks događaja.");
    }
    return static_cast<int>(conflictList[event].size());
}

bool Graph::conflict(int firstEvent, int secondEvent) const {
    if (firstEvent < 0 || secondEvent < 0 ||
        firstEvent >= static_cast<int>(eventsConflict.size()) ||
        secondEvent >= static_cast<int>(eventsConflict.size())) {
        throw out_of_range("Neispravan indeks događaja.");
    }
    return eventsConflict[firstEvent][secondEvent] != 0;
}



// #include<iostream>
// #include "graph.h"

// Graph::Graph(int n) : events_conflict(n, vector<int>(n, 0)), number_of_students(n, 0) {
//     vector<Node> Matrix;
//     for(int i = 0; i < n; ++i){
//         Node A{i};
//         Matrix.push_back(A);
//     }
//     matrix_scheduled = Matrix;
// }

// void Graph::fill_vector(vector<vector<int>> &Table){
//     for(int i = 0; i < Table.size(); ++i){

//         vector<int> slusani_predmeti;

//         for(int j = 0; j < Table[0].size(); ++j){
//             if (Table[i][j] == 1)
//                 slusani_predmeti.push_back(j);
//         }

//         for(int a = 0; a < slusani_predmeti.size(); ++a){
//             number_of_students[slusani_predmeti[a]] += 1;

//             for(int b = a + 1; b < slusani_predmeti.size(); ++b){
//                 int p1 = slusani_predmeti[a];
//                 int p2 = slusani_predmeti[b];

//                 events_conflict[p1][p2] = 1;
//                 events_conflict[p2][p1] = 1;
//             }
//         }
//     }

// }

// int Graph::number_of_conflicts(int i){
//     int k = 0;

//     for (int j = 0; j < events_conflict.size(); ++j)
//         k += events_conflict[i][j];

//     return k;
// }

// bool Graph::is_scheduled(int i){
//     return matrix_scheduled[i].scheduled == true;
// }