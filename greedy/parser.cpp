#include "parser.h"

#include <fstream>
#include <stdexcept> //runtime_error i out_of_range
#include <string>


// pomocna funkcija za citanje cijelog broja pri unosu
static int readInteger(std::istream& input, const std::string& description) {
    int value = 0;

    if (!(input >> value)) { // ako unos nije cijeli broj ili nedostaje prekida se
        throw std::runtime_error("Podatak nedostaje ili nije ispravan: " + description);
    }

    return value;
}

// provjera smije li element matrice biti 0 ili 1
static void requireBinary(int value, const std::string& description) {
    if (value != 0 && value != 1) {
        throw std::runtime_error(description + " mora imati vrijednost 0 ili 1.");
    }
}

// ucitavanje .tim datoteke i spremanje podataka u strukturu TimData
TimData loadTIM(const std::string& filename) {
    std::ifstream file(filename); // otvaranje zadane datoteke
    if (!file) {
        throw std::runtime_error("Ne mogu otvoriti datoteku: " + filename);
    }

    TimData data; // struktura u koju se spremaju podaci procitani iz datoteke
    data.E = readInteger(file, "broj dogadaja");
    data.R = readInteger(file, "broj ucionica");
    data.F = readInteger(file, "broj znacajki");
    data.S = readInteger(file, "broj studenata");

    // provjera ispravnosti podataka u prvoj liniji
    if (data.E <= 0 || data.R <= 0 || data.F < 0 || data.S < 0) {
        throw std::runtime_error("Neispravne vrijednosti u zaglavlju datoteke " + filename);
    }
    
    // ucitava kapacitet svake ucionice
    data.roomSizes.resize(data.R);
    for (int r = 0; r < data.R; ++r) {
        data.roomSizes[r] = readInteger(file, "kapacitet ucionice");
        if (data.roomSizes[r] < 0) {
            throw std::runtime_error("Kapacitet ucionice ne smije biti negativan");
        }
    }

    // studetnEvent[s][e] = 1 ako student 's' pohada dogadaj 'e'
    data.studentEvent.assign(data.S, std::vector<int>(data.E, 0));
    for (int s = 0; s < data.S; ++s) {
        for (int e = 0; e < data.E; ++e) {
            const int value = readInteger(file, "matrica student-dogadaj");
            requireBinary(value, "Element matrice student-dogadaj");
            data.studentEvent[s][e] = value;
        }
    }

    // roomFeature[r][f] = 1 ako ucionica 'r' posjeduje znacajku 'f'
    data.roomFeature.assign(data.R, std::vector<int>(data.F, 0));
    for (int r = 0; r < data.R; ++r) {
        for (int f = 0; f < data.F; ++f) {
            const int value = readInteger(file, "matrica ucionica-znacajka");
            requireBinary(value, "Element matrice ucionica-znacajka");
            data.roomFeature[r][f] = value;
        }
    }

    // eventFeature[e][f] = 1 ako dogadaj 'e' zahtijeva znacajku 'f'
    data.eventFeature.assign(data.E, std::vector<int>(data.F, 0));
    for (int e = 0; e < data.E; ++e) {
        for (int f = 0; f < data.F; ++f) {
            const int value = readInteger(file, "matrica dogadaj-znacajka");
            requireBinary(value, "Element matrice dogadaj-znacajka");
            data.eventFeature[e][f] = value;
        }
    }

    // uvijek 45 termina
    data.eventTimeslot.assign(data.E, std::vector<int>(NUMBER_OF_TIMESLOTS, 0));
    for (int e = 0; e < data.E; ++e) {
        for (int t = 0; t < NUMBER_OF_TIMESLOTS; ++t) {
            const int value = readInteger(file, "matrica dogadaj-termin");
            requireBinary(value, "Element matrice dogadaj-termin");
            data.eventTimeslot[e][t] = value;
        }
    }

    // precedence[first][second] opisuje redoslijed dvaju dogadaja
    data.precedence.assign(data.E, std::vector<int>(data.E, 0));
    for (int first = 0; first < data.E; ++first) {
        for (int second = 0; second < data.E; ++second) {
            const int value = readInteger(file, "matrica prethodnosti");
            if (value < -1 || value > 1) {
                throw std::runtime_error("Element matrice prethodnosti mora biti -1, 0 ili 1");
            }
            data.precedence[first][second] = value;
        }
    }

    // provjera postoji li visak podataka na kraju datoteke
    int extra = 0;
    if (file >> extra) {
        throw std::runtime_error("Datoteka " + filename + " sadrzi visak podataka nakon matrice prethodnosti");
    }

    return data; // vraca potpuno popunjene podatke o instanci
}

// broji koliko je termina dostupno zadanom dogadaju
// zbroj elemenata retka jednak je broju dostupnih termina
int TimData::numberOfAvailableTimeslots(int event) const {
    if (event < 0 || event >= E) { // ne dopusta pristup retku koji ne postoji
        throw std::out_of_range("Neispravan indeks dogadaja");
    }

    // vrijednosti 0 li 1, njihov zbroj je broj termina
    int count = 0;
    for (const int value : eventTimeslot[event]) {
        count += value;
    }
    return count;
}
