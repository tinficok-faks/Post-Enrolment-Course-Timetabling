#include "graph.h"
#include "parser.h"
#include "timetable.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

// Rjesava jedan dataset i ispisuje rezultat u terminal.
Evaluation solveDataset(int datasetNumber, bool printTimetable) {
    string instanceName = "dataset" + to_string(datasetNumber);
    filesystem::path inputFile =
        filesystem::path("datasets") / (instanceName + ".tim");

    TimData data = loadTIM(inputFile.string());

    Graph graph(data.E);
    graph.fillVector(data.studentEvent);

    GreedyTimetabler solver(data, graph);
    Schedule schedule = solver.solve();

    // Provjera je li raspored valjan.
    ValidationResult validation = validateSchedule(data, graph, schedule);

    // Ako nije valjan, pokusavamo ga popraviti uklanjanjem problematicnih dogadaja.
    if (!validation.valid) {
        cout << "Raspored nije valjan. Pokusavam ga popraviti.\n";

        for (const string& error : validation.errors)
            cout << "  - " << error << '\n';

        repairToValid(data, graph, schedule);
        validation = validateSchedule(data, graph, schedule);
    }

    if (!validation.valid)
        throw runtime_error("Raspored nije valjan ni nakon popravka.");

    Evaluation evaluation = evaluateSchedule(data, graph, schedule);

    // ovo ispod je da sprema u outputs folder, zakomentirao sam ga jer je u nekim
    // situacijama imalo problema, a i postao je clutter za pregledavati
    // tijekom debuganinga jer moram gledati u dodatnu datoteku umjesto terminala

    // filesystem::create_directories("outputs");

    // writeSolutionFile(
    //     "outputs/" + instanceName + ".sln",
    //     schedule
    // );

    // writeReadableTimetableFile(
    //     "outputs/raspored_" + instanceName + ".txt",
    //     data,
    //     schedule,
    //     instanceName
    // );

    // citljiv raspored ispisujemo u terminal samo kada se pokrece jedan dataset.
    if (printTimetable) {
        writeReadableTimetable(cout, data, schedule, instanceName);
        cout << endl;
    }

    cout << instanceName << '\n';
    cout << "  udaljenost do dopustivosti: " << evaluation.distanceToFeasibility << '\n';
    cout << "  trosak mekih uvjeta: " << evaluation.softCost << '\n';
    cout << "  ukupan trosak: " << evaluation.totalCost() << '\n';
    cout << "  nerasporedeni dogadaji: " << evaluation.unplacedEvents << "\n\n";

    return evaluation;
}

int main(int argc, char* argv[]) {
    try {
        // Pokretanje svih dataseta: program.exe --all
        if (argc >= 2 && string(argv[1]) == "--all") {
            for (int dataset = 1; dataset <= 24; ++dataset)
                solveDataset(dataset, false);
        }
        else {
            int datasetNumber;

            cout << "Izaberi dataset izmedu 1 i 24: ";
            cin >> datasetNumber;

            if (datasetNumber < 1 || datasetNumber > 24)
                throw invalid_argument(
                    "Broj dataseta mora biti izmedu 1 i 24."
                );

            solveDataset(datasetNumber, true);
        }
    }
    catch (const exception& error) {
        cerr << "Greska: " << error.what() << '\n';
        return 1;
    }

    return 0;
}


// #include <iostream>
// #include "parser.h"
// #include "graph.h"

// using namespace std;

// int main()
// {
//     try
//     {
//         int i;
//         cout << "Izaberi dataset izmedju 1 i 24: ";
//         cin >> i;
//         cout << endl;

//         TimData data = loadTIM("datasets/dataset" + to_string(i) + ".tim");

//         // cout << "Events: " << data.E << endl;
//         // cout << "Rooms: " << data.R << endl;
//         // cout << "Features: " << data.F << endl;
//         // cout << "Students: " << data.S << endl;

//         Graph G {data.E};
//         G.fill_vector(data.studentEvent);
//         for(auto& x : G.matrix_scheduled)
//             cout << x.scheduled << " ";
//         cout << endl;
        
//         // dodatni debug
//         // cout << "Timeslots: " << (data.eventTimeslot.empty() ? 0 : data.eventTimeslot[0].size()) << endl;
//         // cout << "roomSizes[0]=" << (data.roomSizes.empty() ? -1 : data.roomSizes[0]) << endl;
//         // cout << "studentEvent[0][0]=" << data.studentEvent[0][0] << endl;
//         // cout << "eventTimeslot[0][0]=" << (data.eventTimeslot.empty() ? -1 : data.eventTimeslot[0][0]) << endl;

//         // if (!data.precedence.empty())
//         // {
//         //     cout << "precedence[0][1]=" << data.precedence[0][1] << endl;
//         // }

//     }
//     catch (const exception& error)
//     {
//         cerr << "Error: " << error.what() << endl;
//         return 1;
//     }

//     return 0;
// }