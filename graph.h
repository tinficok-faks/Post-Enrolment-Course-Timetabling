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



// #ifndef GRAPH_H

// #define GRAPH_H

// #include <vector>
// using namespace std;

// struct Node {
//     int event_number;
//     bool scheduled;
//     Node (int k) : event_number{k}, scheduled{false} {};
// };

// struct Graph {
//     vector<vector<int>> events_conflict;
//     vector<Node> matrix_scheduled;
//     vector<int> number_of_students;
//     Graph (int n);
    
//     void fill_vector(vector<vector<int>> &Table);
//     int  number_of_conflicts(int i);
//     bool is_scheduled(int i);

//     //dodati f-ju je li timeslot popunjen
// };
// #endif