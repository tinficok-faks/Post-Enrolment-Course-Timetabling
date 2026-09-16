#include "filereader.h"
#include "tabu_search.h"
#include "output.h"
#include "local_search.h"

#include <iostream>

int main(){

    try{
        int datasetNumber;
        int methodNumber;
    
        std::cout << "Izaberi dataset izmedu 1 i 24: ";
        std::cin >> datasetNumber;

        std::cout << "Izaberi BIN Transfer (1), FIN Transfer (2), BIN Swap (3), FIN Swap (4) ili -1 ako zelite Tabu Search rjesenje: ";
        std::cin >> methodNumber;
    
        if (datasetNumber < 1 || datasetNumber > 24)
            throw std::invalid_argument("Broj dataseta mora biti izmedu 1 i 24.");


        if ( methodNumber < 1 || methodNumber > 4) {
            throw std::invalid_argument("Broj metode mora biti 1, 2, 3 ili 4.");
        }
    
        EventData data = readfiles(std::to_string(datasetNumber));
        

        std::string inputSolution = "../greedy_outputs/raspored_dataset" + std::to_string(datasetNumber) + "_greedy.sln";
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

            best_improving_neighbor_transfer(schedule);

            writeSolution(
                "../ls_outputs/ls1_outputs/ls1_output" + std::to_string(datasetNumber) + ".sln",
                schedule.numberOfEvents,
                schedule.placedEvents,
                datasetNumber
            );
        }
        else if (methodNumber == 2){

            first_improving_neighbor_transfer(schedule);

            writeSolution(
                "../ls_outputs/ls2_outputs/ls2_output" + std::to_string(datasetNumber) + ".sln",
                schedule.numberOfEvents,
                schedule.placedEvents,
                datasetNumber
            );

        }
        else if (methodNumber == 3){
            best_improving_neighbor_swap(schedule);

            writeSolution(
                "../ls_outputs/ls1_outputs/ls1_output" + std::to_string(datasetNumber) + ".sln",
                schedule.numberOfEvents,
                schedule.placedEvents,
                datasetNumber
            );
        }
        else{
            first_improving_neighbor_swap(schedule);

            writeSolution(
                "../ls_outputs/ls2_outputs/ls2_output" + std::to_string(datasetNumber) + ".sln",
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