#ifndef GRAPH_H

#define GRAPH_H

#include <vector>

struct Graph {
    //matrica konflikata dogadaja (ako 2 predmeta slusa isti student, onda ne mogu biti u istom terminu)
    //1 ako su u konfliktu, inace 0
    std::vector<std::vector<int>> eventsConflict;
    //svaki predmet ima listu predmeta s kojima je u konfliktu
    std::vector<std::vector<int>> conflictList;
    //biljezi studente koji slusaju pojedini predmet
    std::vector<std::vector<int>> studentsOfEvent;

    explicit Graph(int numberOfEvents);

    void fillVector(const std::vector<std::vector<int>>& studentEvent);
    int numberOfConflicts(int event) const;
    bool conflict(int firstEvent, int secondEvent) const;
};

#endif
