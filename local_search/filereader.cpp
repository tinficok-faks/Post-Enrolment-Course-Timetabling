#include "filereader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

EventData readfiles(std::string datasetNumber) {
	std::ifstream file("../file_readings/data.sln");
	if (!file) {
		throw std::runtime_error("Ne mogu otvoriti datoteku data.sln");
	}

	std::vector<std::vector<std::vector<int>>> objects;
	std::vector<std::vector<int>> currentObject;
	std::string line;

	auto readRow = [](const std::string& text) {
		std::istringstream lineStream(text);
		std::vector<int> row;
		int value = 0;

		while (lineStream >> value) {
			row.push_back(value);
		}

		if (row.empty() || !lineStream.eof()) {
			throw std::runtime_error("Redak datoteke sadrzi neispravan podatak: " + text);
		}

		return row;
	};

	auto finishObject = [&]() {
		if (!currentObject.empty()) {
			objects.push_back(currentObject);
			currentObject.clear();
		}
	};

	while (std::getline(file, line)) {
		std::size_t start = 0;
		while (start <= line.size()) {
			const std::size_t separator = line.find(';', start);
			const std::string part = line.substr(
				start,
				separator == std::string::npos ? std::string::npos : separator - start);

			if (!part.empty() && part.find_first_not_of(" \t\r") != std::string::npos) {
				currentObject.push_back(readRow(part));
			}

			if (separator == std::string::npos) {
				break;
			}

			finishObject();
			start = separator + 1;
		}
	}
	finishObject();

	if (objects.size() < 8
		|| objects[0].size() != 1
		|| objects[0][0].size() != 1) {
		throw std::runtime_error(
			"data.sln mora poceti brojem studenata S u zasebnom bloku");
	}

	EventData data;
	data.S = objects[0][0][0];
	if (data.S < 0) {
		throw std::runtime_error("Broj studenata S ne smije biti negativan");
	}

	for (const std::vector<int>& row : objects[1]) {
		data.roomSizes.insert(data.roomSizes.end(), row.begin(), row.end());
	}
	data.conflictList = std::move(objects[2]);
	data.studentsOfEvent = std::move(objects[3]);
	data.roomFeature = std::move(objects[4]);
	data.eventFeature = std::move(objects[5]);
	data.eventTimeslot = std::move(objects[6]);
	data.precedence = std::move(objects[7]);


	data.placedEvents.assign(45, std::vector<int>(data.roomSizes.size(), -1));
	std::ifstream timetableFile("../greedy_outputs/raspored_dataset" + datasetNumber + "_greedy.sln");
	if (!timetableFile) {
		throw std::runtime_error("Ne mogu otvoriti datoteku raspored_greedy.sln");
	}

	int event = 0;
	while (std::getline(timetableFile, line)) {
		if (line.empty()) {
			continue;
		}

		std::istringstream lineStream(line);
		int timeslot = 0;
		int room = 0;

		if (!(lineStream >> timeslot >> room)) {
			throw std::runtime_error("Redak rasporeda sadrzi neispravan podatak: " + line);
		}

		lineStream >> std::ws;
		if (lineStream.peek() != std::char_traits<char>::eof()) {
			throw std::runtime_error("Redak rasporeda sadrzi neispravan podatak: " + line);
		}

		if (timeslot == -1 && room == -1) {
			data.unplacedEvents.push_back(event);
		} else if (event >= 0) {
			if (timeslot < 0 || timeslot >= 45
				|| room < 0 || room >= static_cast<int>(data.roomSizes.size())) {
				throw std::runtime_error("Raspored sadrzi neispravan timeslot ili room: " + line);
			}

			data.placedEvents[timeslot][room] = event;
		}

		event++;
	}

	return data;
}

void loadInputSolution(const std::string& filename, EventData& data){
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error(
            "Nije moguce otvoriti pocetno rjesenje: " + filename
        );
    }

    // resetiraj raspored
    for (auto& timeslot : data.placedEvents) {
        std::fill(timeslot.begin(), timeslot.end(), -1);
    }

    data.unplacedEvents.clear();

    int numberOfEvents = static_cast<int>(data.studentsOfEvent.size());

    for (int event = 0; event < numberOfEvents; ++event) {

        int timeslot;
        int room;

        if (!(file >> timeslot >> room)) {
            throw std::runtime_error(
                "Nedostaje zapis za event " +
                std::to_string(event) +
                " u " + filename
            );
        }

        // nerasporeden event
        if (timeslot == -1 && room == -1) {
            data.unplacedEvents.push_back(event);
            continue;
        }

        // provjera timeslota
        if (timeslot < 0 
			|| timeslot >= static_cast<int>(data.placedEvents.size())
        ) {
            throw std::runtime_error(
                "Neispravan timeslot za event " +
                std::to_string(event)
            );
        }

        // provjera sobe
        if (room < 0 
			|| room >= static_cast<int>(data.placedEvents[timeslot].size())
        ) {
            throw std::runtime_error(
                "Neispravna soba za event " +
                std::to_string(event)
            );
        }

        if (data.placedEvents[timeslot][room] != -1) {
            throw std::runtime_error(
                "Dvije aktivnosti koriste istu sobu u timeslotu " +
                std::to_string(timeslot)
            );
        }

        data.placedEvents[timeslot][room] = event;
    }
}