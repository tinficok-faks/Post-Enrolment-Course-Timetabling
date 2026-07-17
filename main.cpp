#include "graph.h"
#include "parser.h"
#include "timetable.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>


// Rjesava jedan dataset i ispisuje rezultat u terminal.
Evaluation solveDataset(int datasetNumber, bool printTimetable) {
    std::string instanceName = "dataset" + std::to_string(datasetNumber);
    std::filesystem::path inputFile =
        std::filesystem::path("datasets") / (instanceName + ".tim");

    TimData data = loadTIM(inputFile.string());

    Graph graph(data.E);
    graph.fillVector(data.studentEvent);

    GreedyTimetabler solver(data, graph);
    Schedule schedule = solver.solve();

    // Provjera je li raspored valjan.
    ValidationResult validation = validateSchedule(data, graph, schedule);

    // Ako nije valjan, pokusavamo ga popraviti uklanjanjem problematicnih dogadaja.
    if (!validation.valid) {
        std::cout << "Raspored nije valjan. Pokusavam ga popraviti.\n";

        for (const std::string& error : validation.errors)
            std::cout << "  - " << error << '\n';

        repairToValid(data, graph, schedule);
        validation = validateSchedule(data, graph, schedule);
    }

    if (!validation.valid)
        throw std::runtime_error("Raspored nije valjan ni nakon popravka.");

    Evaluation evaluation = evaluateSchedule(data, graph, schedule);

    // ovo ispod je da sprema u outputs folder, zakomentirao sam ga jer je u nekim
    // situacijama imalo problema, a i postao je clutter za pregledavati
    // tijekom debuganinga jer moram gledati u dodatnu datoteku umjesto terminala

    // std::filesystem::create_directories("outputs");

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
        writeReadableTimetable(std::cout, data, schedule, instanceName);
        std::cout << std::endl;
    }

    std::cout << instanceName << '\n';
    std::cout << "  udaljenost do dopustivosti: " << evaluation.distanceToFeasibility << '\n';
    std::cout << "  trosak mekih uvjeta: " << evaluation.softCost << '\n';
    std::cout << "  ukupan trosak: " << evaluation.totalCost() << '\n';
    std::cout << "  nerasporedeni dogadaji: " << evaluation.unplacedEvents << "\n\n";

    return evaluation;
}

int main() {
    try{
        int datasetNumber;
    
        std::cout << "Izaberi dataset izmedu 1 i 24: ";
        std::cin >> datasetNumber;
    
        if (datasetNumber < 1 || datasetNumber > 24)
            throw std::invalid_argument("Broj dataseta mora biti izmedu 1 i 24.");
    
        solveDataset(datasetNumber, true);
    }
    catch (const std::exception& error) {
        std::cerr << "Greska: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
