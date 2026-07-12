#include<iostream>
#include "graph.h"


Graph::Graph(int n) : events_conflict(n, vector<int>(n, 0)), number_of_students(n, 0) {
    vector<Node> Matrix;
    for(int i = 0; i < n; ++i){
        Node A{i};
        Matrix.push_back(A);
    }
    matrix_scheduled = Matrix;
}

void Graph::fill_vector(vector<vector<int>> &Table){
    for(int i = 0; i < Table.size(); ++i){

        vector<int> slusani_predmeti;

        for(int j = 0; j < Table[0].size(); ++j){
            if (Table[i][j] == 1)
                slusani_predmeti.push_back(j);
        }

        for(int a = 0; a < slusani_predmeti.size(); ++a){
            number_of_students[slusani_predmeti[a]] += 1;

            for(int b = a + 1; b < slusani_predmeti.size(); ++b){
                int p1 = slusani_predmeti[a];
                int p2 = slusani_predmeti[b];

                events_conflict[p1][p2] = 1;
                events_conflict[p2][p1] = 1;
            }
        }
    }

}

int Graph::number_of_conflicts(int i){
    int k = 0;

    for (int j = 0; j < events_conflict.size(); ++j)
        k += events_conflict[i][j];

    return k;
}

bool Graph::is_scheduled(int i){
    return matrix_scheduled[i].scheduled == true;
}