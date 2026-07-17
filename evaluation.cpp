#include "timetable.h"

#include <bit>
#include <stdexcept>


static int countBits(unsigned int value) {
#if __cplusplus >= 202002L // ovo je ako compileamo s std=c++20
    return std::popcount(value);
#else
    return __builtin_popcount(value); // a ovo je ako po std=c++17
#endif
}


Evaluation evaluateSchedule(const TimData& data,
                            const Graph& graph,
                            const Schedule& schedule) {
    const ValidationResult validation = validateSchedule(data, graph, schedule, 1);
    if (!validation.valid) {
        throw std::invalid_argument("Nije moguće evaluirati raspored koji krši čvrste uvjete.");
    }

    Evaluation evaluation;
    std::vector<unsigned long long> studentMask(data.S, 0ULL);

    for (int event = 0; event < data.E; ++event) {
        if (!schedule[event].isPlaced()) {
            ++evaluation.unplacedEvents;
            evaluation.distanceToFeasibility += graph.studentsOfEvent[event].size();
            continue;
        }
        for (const int student : graph.studentsOfEvent[event]) {
            studentMask[student] |= (1ULL << schedule[event].timeslot);
        }
    }

    auto dayPenalty = [](unsigned int mask) {
        const int classes = countBits(mask);
        int penalty = (classes == 1) ? 1 : 0;
        if ((mask & (1U << (SLOTS_PER_DAY - 1))) != 0U) {
            ++penalty;
        }

        int run = 0;
        for (int slot = 0; slot < SLOTS_PER_DAY; ++slot) {
            if ((mask & (1U << slot)) != 0U) {
                ++run;
            } else {
                if (run >= 3) {
                    penalty += run - 2;
                }
                run = 0;
            }
        }
        if (run >= 3) {
            penalty += run - 2;
        }
        return penalty;
    };

    const unsigned int dayMask = (1U << SLOTS_PER_DAY) - 1U;
    for (int student = 0; student < data.S; ++student) {
        for (int day = 0; day < NUMBER_OF_DAYS; ++day) {
            const unsigned int mask = static_cast<unsigned int>(
                (studentMask[student] >> (day * SLOTS_PER_DAY)) & dayMask);
            evaluation.softCost += dayPenalty(mask);
        }
    }

    return evaluation;
}
