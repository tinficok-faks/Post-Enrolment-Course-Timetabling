#include "timetable.h"


#include <filesystem>
#include <fstream>
#include <stdexcept>

static void createParentFolder(const std::string& filename) {
    std::filesystem::path path(filename);
    if (path.has_parent_path())
        std::filesystem::create_directories(path.parent_path());
}

void writeSolutionFile(const std::string& filename, const Schedule& schedule) {
    createParentFolder(filename);
    std::ofstream output(filename);
    if (!output) {
        throw std::runtime_error("Ne mogu otvoriti izlaznu datoteku: " + filename);
    }

    for (const Assignment assignment : schedule) {
        output << assignment.timeslot << ' ' << assignment.room << '\n';
    }
}

void writeReadableTimetable(std::ostream& output,
                            const TimData& data,
                            const Schedule& schedule) {
    
    std::vector<int> eventTimeslot(data.E, -1);
    std::vector<int> eventRoom(data.E, -1);

    for (int event = 0; event < data.E; ++event) {
        if (schedule[event].isPlaced()) {
            eventTimeslot[event] = schedule[event].timeslot;
            eventRoom[event] = schedule[event].room;
        } 
    }


    for (int event = 0; event < data.E; ++event){
        output << eventTimeslot[event] << ' '
            << eventRoom[event] << '\n';
    }
}

void writeReadableTimetableFile(const std::string& filename,
                                const TimData& data,
                                const Schedule& schedule
                            ) {
    createParentFolder(filename);
    std::ofstream output(filename);
    if (!output) {
        throw std::runtime_error("Ne mogu otvoriti izlaznu datoteku: " + filename);
    }
    writeReadableTimetable(output, data, schedule);
}


template <typename T>
void printContainer(std::ofstream& output, const T& container) {
    for (const auto& x : container) {
        output << x << " ";
    }
    output << "\n";
}

void writeEventsdata(const TimData& data, const Graph& graph){
    const std::string filename = "../file_readings/data.txt";
    createParentFolder(filename);
    std::ofstream output(filename);
    if (!output) {
        throw std::runtime_error("Ne mogu otvoriti izlaznu datoteku: " + filename);
    }

    output << data.S <<";\n";

    printContainer(output, data.roomSizes);
    output << ";\n";

    for(auto& x : graph.conflictList){
        printContainer(output, x);
    }
    output << ";\n";

    for(auto& x : graph.studentsOfEvent){
        printContainer(output, x);
    }
    output << ";\n";

    for(auto& x : data.roomFeature){
        printContainer(output, x);
    }
    output << ";\n";

    for(auto& x : data.eventFeature){
        printContainer(output, x);
    }
    output << ";\n";

    for(auto& x : data.eventTimeslot){
        printContainer(output, x);
    }
    output << ";\n";

    for(auto& x : data.precedence){
        printContainer(output, x);
    }
    output << ";\n"; 
}
