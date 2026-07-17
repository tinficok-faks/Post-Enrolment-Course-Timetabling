#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>


constexpr int NUMBER_OF_DAYS = 5; // 5 radnih dana
constexpr int SLOTS_PER_DAY = 9; // 9 predavanja dnevno
constexpr int NUMBER_OF_TIMESLOTS = NUMBER_OF_DAYS * SLOTS_PER_DAY; // 45 termina tjedno/ukupno

struct TimData {
    int E = 0; // broj dogadaja
    int R = 0; // broj ucionica
    int F = 0; // broj znacajki
    int S = 0; // broj studenata

    std::vector<int> roomSizes; // kapacitet ucionice r
    std::vector<std::vector<int>> studentEvent; // studentEvent[s][e] = 1 ako student 's' pohada dogadaj 'e'; ako ne onda 0
    std::vector<std::vector<int>> roomFeature; // roomFeature[r][f] = 1 ako ucionica 'r' posjeduje znacajku 'f'; ako ne onda 0 
    std::vector<std::vector<int>> eventFeature; // eventFeature[e][f] = 1 ako dogadaj 'e' zahtijeva znacajku 'f'; ako ne onda 0
    std::vector<std::vector<int>> eventTimeslot; // eventTimeslot[e][t] = 1 znaci termin je dostupan; ako nije onda 0
    std::vector<std::vector<int>> precedence; // precedence[first][second] opisuje odnos redoslijeda:
                                              // 1 - prvi dogadaj mora biti prije drugoga
                                              // -1 - prvi dogadaj mora biti poslije drugoga
                                              // 0 - nema zahtjeva za redoslijed

    int numberOfAvailableTimeslots(int event) const; // broj termina u koje se zadani dogadaj smije rasporediti
                                                     // const jer nikad ne mijenja podatke objekta TimData
};

// ucitava .tim datoteke i vraca ispunjen objekt TimData
TimData loadTIM(const std::string& filename);

#endif
