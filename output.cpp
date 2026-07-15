#include "timetable.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <stdexcept>

using namespace std;

static void createParentFolder(const string& filename) {
    filesystem::path path(filename);
    if (path.has_parent_path())
        filesystem::create_directories(path.parent_path());
}

void writeSolutionFile(const string& filename, const Schedule& schedule) {
    createParentFolder(filename);
    ofstream output(filename);
    if (!output) {
        throw runtime_error("Ne mogu otvoriti izlaznu datoteku: " + filename);
    }

    for (const Assignment assignment : schedule) {
        output << assignment.timeslot << ' ' << assignment.room << '\n';
    }
}

void writeReadableTimetable(ostream& output,
                            const TimData& data,
                            const Schedule& schedule,
                            const string& instanceName) {
    vector<vector<int>> roomEvent(
        NUMBER_OF_TIMESLOTS, vector<int>(data.R, -1));
    vector<int> unplaced;

    for (int event = 0; event < data.E; ++event) {
        if (schedule[event].isPlaced()) {
            roomEvent[schedule[event].timeslot][schedule[event].room] = event;
        } else {
            unplaced.push_back(event);
        }
    }

    output << "Raspored za " << instanceName << "\n";
    output << "Događaji su označeni slovom E i indeksom iz ulazne datoteke.\n\n";

    const array<string, NUMBER_OF_DAYS> dayNames = {
        "PON", "UTO", "SRI", "CET", "PET"};

    for (int room = 0; room < data.R; ++room) {
        output << "Ucionica " << room << " (kapacitet " << data.roomSizes[room] << ")\n";
        output << left << setw(7) << "sat";
        for (const string& day : dayNames) {
            output << setw(10) << day;
        }
        output << '\n';

        for (int hour = 0; hour < SLOTS_PER_DAY; ++hour) {
            output << left << setw(7) << hour;
            for (int day = 0; day < NUMBER_OF_DAYS; ++day) {
                const int timeslot = day * SLOTS_PER_DAY + hour;
                const int event = roomEvent[timeslot][room];
                const string cell = (event == -1) ? "-" : "E" + to_string(event);
                output << setw(10) << cell;
            }
            output << '\n';
        }
        output << '\n';
    }

    output << "Neraspoređeni događaji (" << unplaced.size() << "):";
    if (unplaced.empty()) {
        output << " nema";
    } else {
        for (const int event : unplaced) {
            output << ' ' << event;
        }
    }
    output << "\n";
}

void writeReadableTimetableFile(const string& filename,
                                const TimData& data,
                                const Schedule& schedule,
                                const string& instanceName) {
    createParentFolder(filename);
    ofstream output(filename);
    if (!output) {
        throw runtime_error("Ne mogu otvoriti izlaznu datoteku: " + filename);
    }
    writeReadableTimetable(output, data, schedule, instanceName);
}
