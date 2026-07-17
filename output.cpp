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
                            const Schedule& schedule,
                            const std::string& instanceName) {
    std::vector<std::vector<int>> roomEvent(
        NUMBER_OF_TIMESLOTS, std::vector<int>(data.R, -1));
    std::vector<int> unplaced;

    for (int event = 0; event < data.E; ++event) {
        if (schedule[event].isPlaced()) {
            roomEvent[schedule[event].timeslot][schedule[event].room] = event;
        } else {
            unplaced.push_back(event);
        }
    }

    output << "Raspored za " << instanceName << "\n";
    output << "Dogadaji su oznaceni slovom E i indeksom iz ulazne datoteke.\n\n";

    const std::vector<std::string> dayNames = {"PON", "UTO", "SRI", "CET", "PET"};

    for (int room = 0; room < data.R; ++room) {
        output << "Ucionica " << room << " (kapacitet " << data.roomSizes[room] << ")\n";
        output << std::left << std::setw(7) << "sat";
        for (auto& day : dayNames) {
            output << std::setw(10) << day;
        }
        output << '\n';

        for (int hour = 0; hour < SLOTS_PER_DAY; ++hour) {
            output << std::left << std::setw(7) << hour;
            for (int day = 0; day < NUMBER_OF_DAYS; ++day) {
                const int timeslot = day * SLOTS_PER_DAY + hour;
                const int event = roomEvent[timeslot][room];
                const std::string cell = (event == -1) ? "-" : "E" + std::to_string(event);
                output << std::setw(10) << cell;
            }
            output << '\n';
        }
        output << '\n';
    }

    output << "Nerasporedeni dogadaji (" << unplaced.size() << "):";
    if (unplaced.empty()) {
        output << " nema";
    } else {
        for (const int event : unplaced) {
            output << ' ' << event;
        }
    }
    output << "\n";
}

void writeReadableTimetableFile(const std::string& filename,
                                const TimData& data,
                                const Schedule& schedule,
                                const std::string& instanceName) {
    createParentFolder(filename);
    std::ofstream output(filename);
    if (!output) {
        throw std::runtime_error("Ne mogu otvoriti izlaznu datoteku: " + filename);
    }
    writeReadableTimetable(output, data, schedule, instanceName);
}
