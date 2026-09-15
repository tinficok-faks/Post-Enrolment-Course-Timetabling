#include "filereader.h"
#include "tabu_search.h"
#include "output.h"
#include "local_search.h"

#include <iostream>

void createTabuSolution(int datasetNumber) {
    EventData data = readfiles(std::to_string(datasetNumber));

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

    writeSolution(
        "../tabu_outputs/ts_output" + std::to_string(datasetNumber) + ".sln",
        ts.numberOfEvents,
        ts.placedEvents,
        datasetNumber
    );

}


int main(){

    try{
        int datasetNumber;
        int methodNumber;
    
        std::cout << "Izaberi dataset izmedu 1 i 24: ";
        std::cin >> datasetNumber;

        std::cout << "Izaberi BIN (1), FIN (2) ili -1 ako zelite Tabu Search rjesenje: ";
        std::cin >> methodNumber;
    
        if (datasetNumber < 1 || datasetNumber > 24)
            throw std::invalid_argument("Broj dataseta mora biti izmedu 1 i 24.");

        if(methodNumber == -1){
            createTabuSolution(datasetNumber);
            return 0;
        }

        if (methodNumber != 1 && methodNumber != 2) {
            throw std::invalid_argument("Broj metode mora biti 1 ili 2.");
        }
    
        EventData data = readfiles(std::to_string(datasetNumber));
        
        std::string inputSolution = "../tabu_outputs/ts_output" + std::to_string(datasetNumber) + ".sln";
        loadInputSolution(inputSolution, data);

        TabuSearch schedule(
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

        if (methodNumber == 1) {

            best_improving_neighbor(schedule);

            writeSolution(
                "../ls_outputs/ls1_outputs/ls1_output" + std::to_string(datasetNumber)     + ".sln",
                schedule.numberOfEvents,
                schedule.placedEvents,
                datasetNumber
            );
        }
        else{

            first_improving_neighbor(schedule);

            writeSolution(
                "../ls_outputs/ls2_outputs/ls2_output" + std::to_string(datasetNumber)     + ".sln",
                schedule.numberOfEvents,
                schedule.placedEvents,
                datasetNumber
            );

        }


    }
    catch (const std::exception& error) {
        std::cerr << "Greska: " << error.what() << '\n';
        return 1;
    }

    return 0;
}