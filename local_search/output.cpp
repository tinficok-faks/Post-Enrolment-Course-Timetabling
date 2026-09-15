#include "output.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

static void createParentFolder(const std::string& filename) {
    std::filesystem::path path(filename);
    if (path.has_parent_path())
        std::filesystem::create_directories(path.parent_path());
}

void writeSolution(
    const std::string& fileName,
    int numberOfEvents,
    const std::vector<std::vector<int>>& placedEvents,
    int number) 
{
    createParentFolder(fileName);
    std::ofstream output(fileName);

    if (!output.is_open()) {
        std::cerr << "Nije moguce otvoriti output datoteku.\n";
        return;
    }

    // za svaki event spremamo njegov timeslot i room
    // -1 -1 znaci da event nije rasporeden
    // kako je i zadano za validaciju
    std::vector<int> eventTimeslot(numberOfEvents, -1);
    std::vector<int> eventRoom(numberOfEvents, -1);

    int numberOfTimeslots = static_cast<int>(placedEvents.size());

    if (numberOfTimeslots == 0) {
        return;
    }

    int numberOfRooms = static_cast<int>(placedEvents[0].size());


    // placedEvents[timeslot][room] = event
    // pretvaramo u:
    // eventTimeslot[event] = timeslot
    // eventRoom[event] = room
    for (int timeslot = 0; timeslot < numberOfTimeslots; ++timeslot)
    {
        for (int room = 0; room < numberOfRooms; ++room)
        {
            int event = placedEvents[timeslot][room];

            if (event == -1) {
                continue;
            }

            eventTimeslot[event] = timeslot;
            eventRoom[event] = room;
        }
    }


    // BITNO:
    // eventi se ispisuju redom:
    // event 0
    // event 1
    // ...
    //
    for (int event = 0; event < numberOfEvents; ++event)
    {
        output << eventTimeslot[event] << ' '
            << eventRoom[event] << '\n';
    }
        const std::filesystem::path datasetsDirectory = "../datasets";
        const std::string instanceName = std::to_string(number);
        std::filesystem::path sourceTimetable;

        for (const std::filesystem::directory_entry& entry :
             std::filesystem::directory_iterator(datasetsDirectory)) {
            if (entry.is_regular_file()
                && entry.path().extension() == ".tim"
                && entry.path().stem().string() == "dataset" + instanceName) {
                sourceTimetable = entry.path();
                break;
            }
        }

        if (sourceTimetable.empty()) {
            throw std::runtime_error(
                "Ne mogu pronaci dataset za instanceName " + instanceName);
        }

        const std::filesystem::path outputDirectory = "../ls1_outputs";
        std::filesystem::create_directories(outputDirectory);
        const std::filesystem::path outputTimetable =
            outputDirectory / ("ts_output" + instanceName + ".tim");
        std::filesystem::remove(outputTimetable);
        std::filesystem::copy_file(sourceTimetable, outputTimetable);
}