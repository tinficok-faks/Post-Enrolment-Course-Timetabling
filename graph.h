#ifndef GRAPH_H

#define GRAPH_H

#include <vector>
using namespace std;

struct Graph {
    vector<std::vector<unsigned char>> eventsConflict;
    vector<std::vector<int>> conflictList;
    vector<std::vector<int>> studentsOfEvent;
    vector<int> numberOfStudents;

    explicit Graph(int numberOfEvents);

    void fillVector(const vector<vector<int>>& studentEvent);
    int numberOfConflicts(int event) const;
    bool conflict(int firstEvent, int secondEvent) const;
};

#endif
