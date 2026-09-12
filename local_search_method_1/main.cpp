#include "filereader.h"
#include "tabu_search.h"
#include "output.h"
#include "local_search.h"

#include <iostream>

int main(){



    try{
        int datasetNumber;
    
        std::cout << "Izaberi dataset izmedu 1 i 24 koji ima greedy raspored (samo zadnji koji ste napravili s greedy): ";
        std::cin >> datasetNumber;
    
        if (datasetNumber < 1 || datasetNumber > 24)
            throw std::invalid_argument("Broj dataseta mora biti izmedu 1 i 24.");
    
        EventData data;
        data = readfiles(std::to_string(datasetNumber));
        
        TabuSearch ts(
            data.roomSizes,
            data.placedEvents,
            data.unplacedEvents,
            data.conflictList,
            data.studentsOfEvent,
            data.roomFeature,
            data.eventFeature,
            data.eventTimeslot,
            data.precedence,
            data.S
        );
    
        ts.solve(200000);

        best_improving_neighbor(ts);

        writeSolution(
            "../ls1_outputs/ts_output" + std::to_string(datasetNumber) + ".sln",
            ts.numberOfEvents,
            ts.placedEvents,
            datasetNumber
        );

    }
    catch (const std::exception& error) {
        std::cerr << "Greska: " << error.what() << '\n';
        return 1;
    }

    return 0;
}