#include "parser.h"

#include <fstream>
#include <stdexcept>
#include <string>

using namespace std;

static int readInteger(istream& input, const string& description) {
    int value = 0;

    if (!(input >> value)) {
        throw runtime_error("Nedostaje ili nije ispravan podatak: " + description);
    }

    return value;
}

static void requireBinary(int value, const string& description) {
    if (value != 0 && value != 1) {
        throw runtime_error(description + " mora imati vrijednost 0 ili 1.");
    }
}

TimData loadTIM(const string& filename) {
    ifstream file(filename);
    if (!file) {
        throw runtime_error("Ne mogu otvoriti datoteku: " + filename);
    }

    TimData data;
    data.E = readInteger(file, "broj dogadaja");
    data.R = readInteger(file, "broj ucionica");
    data.F = readInteger(file, "broj znacajki");
    data.S = readInteger(file, "broj studenata");

    if (data.E <= 0 || data.R <= 0 || data.F < 0 || data.S < 0) {
        throw runtime_error("Neispravne vrijednosti u zaglavlju datoteke " + filename + ".");
    }
    if (data.R > 32) {
        throw runtime_error("Ova implementacija podržava najviše 32 ucionice.");
    }

    data.roomSizes.resize(data.R);
    for (int r = 0; r < data.R; ++r) {
        data.roomSizes[r] = readInteger(file, "kapacitet ucionice");
        if (data.roomSizes[r] < 0) {
            throw runtime_error("Kapacitet ucionice ne smije biti negativan.");
        }
    }

    data.studentEvent.assign(data.S, vector<int>(data.E, 0));
    for (int s = 0; s < data.S; ++s) {
        for (int e = 0; e < data.E; ++e) {
            const int value = readInteger(file, "matrica student-dogadaj");
            requireBinary(value, "Element matrice student-dogadaj");
            data.studentEvent[s][e] = value;
        }
    }

    data.roomFeature.assign(data.R, vector<int>(data.F, 0));
    for (int r = 0; r < data.R; ++r) {
        for (int f = 0; f < data.F; ++f) {
            const int value = readInteger(file, "matrica ucionica-znacajka");
            requireBinary(value, "Element matrice ucionica-znacajka");
            data.roomFeature[r][f] = value;
        }
    }

    data.eventFeature.assign(data.E, vector<int>(data.F, 0));
    for (int e = 0; e < data.E; ++e) {
        for (int f = 0; f < data.F; ++f) {
            const int value = readInteger(file, "matrica dogadaj-znacajka");
            requireBinary(value, "Element matrice dogadaj-znacajka");
            data.eventFeature[e][f] = value;
        }
    }

    // U ITC 2007 Track 2 modelu uvijek postoji 45 termina.
    data.eventTimeslot.assign(data.E, vector<int>(NUMBER_OF_TIMESLOTS, 0));
    for (int e = 0; e < data.E; ++e) {
        for (int t = 0; t < NUMBER_OF_TIMESLOTS; ++t) {
            const int value = readInteger(file, "matrica dogadaj-termin");
            requireBinary(value, "Element matrice dogadaj-termin");
            data.eventTimeslot[e][t] = value;
        }
    }

    data.precedence.assign(data.E, vector<int>(data.E, 0));
    for (int first = 0; first < data.E; ++first) {
        for (int second = 0; second < data.E; ++second) {
            const int value = readInteger(file, "matrica prethodnosti");
            if (value < -1 || value > 1) {
                throw runtime_error("Element matrice prethodnosti mora biti -1, 0 ili 1.");
            }
            data.precedence[first][second] = value;
        }
    }

    int extra = 0;
    if (file >> extra) {
        throw runtime_error("Datoteka " + filename + " sadrži višak podataka nakon matrice prethodnosti.");
    }

    return data;
}

int TimData::numberOfAvailableTimeslots(int event) const {
    if (event < 0 || event >= E) {
        throw out_of_range("Neispravan indeks dogadaja.");
    }

    int count = 0;
    for (const int value : eventTimeslot[event]) {
        count += value;
    }
    return count;
}


// #include <sstream>
// #include <iostream>
// #include <vector>
// #include <fstream>
// #include <string>

// #include "parser.h"
// using namespace std;

// /*
//  * Ucitava podatke iz .tim datoteke i sprema ih u strukturu TimData.
//  *
//  * Ocekivani redoslijed podataka u datoteci je:
//  *  1. E, R, F, S
//  *  2. kapaciteti prostorija
//  *  3. matrica student–dogadaj
//  *  4. matrica prostorija–znacajka
//  *  5. matrica dogadaj–znacajka
//  *  6. matrica dogadaj–termin
//  *  7. opcionalna matrica prethodnosti dogadaja
//  */
// TimData loadTIM(const string &filename)
// {
//     // Otvara ulaznu datoteku ciji je naziv ili put predan funkciji.
//     ifstream file(filename);

//     // Ako datoteku nije moguce otvoriti, prekida ucitavanje iznimkom.
//     if (!file.is_open())
//     {
//         throw runtime_error("Ne mogu otvoriti datoteku.");
//     }

    
//     // Ucitava cijeli sadržaj datoteke u jedan string.
//     // Stringstream privremeno prima sadržaj datoteke preko file.rdbuf().
//     string text;
//     {
//         stringstream ss;
//         ss << file.rdbuf();
//         text = ss.str();
//     }

//     /*
//      * Iz teksta izdvaja samo tokene koji u cijelosti predstavljaju cijele brojeve.
//      *
//      * v sadrži sve pronadene brojeve redom kojim se pojavljuju u datoteci.
//      * long long se koristi tijekom ucitavanja kako bi se smanjio rizik od
//      * prekoracenja prije pretvorbe u int.
//      */
//     vector<long long> values;
//     string token;
//     stringstream ss(text);
//     while (ss >> token)
//     {
//         size_t pos;
//         // stoll pretvara pocetni brojcani dio tokena u long long.
//         long long v = stoll(token, &pos);
//         // Token se prihvaca samo ako je cijeli token uspješno pretvoren u broj.
//         if (pos == token.size())
//             values.push_back(v);
//     }

//     /*
//      * Prva cetiri broja moraju postojati jer predstavljaju zaglavlje:
//      * E - broj dogadaja
//      * R - broj prostorija
//      * F - broj znacajki prostorija i dogadaja
//      * S - broj studenata
//      */
//     if (values.size() < 4)
//     {
//         throw runtime_error("Input file is too short after filtering tokens.");
//     }

//     // idx oznacava poziciju sljedeceg broja koji treba procitati iz values.
//     size_t idx = 0;

//     // Struktura u koju se spremaju svi ucitani podaci.
//     TimData data;
//     // citanje cetiriju vrijednosti zaglavlja.
//     data.E = static_cast<int>(values[idx++]);
//     data.R = static_cast<int>(values[idx++]);
//     data.F = static_cast<int>(values[idx++]);
//     data.S = static_cast<int>(values[idx++]);


//     // Provjerava osnovnu ispravnost zaglavlja.
//     // Broj dogadaja mora biti pozitivan, dok ostale vrijednosti ne smiju biti negativne.
//     if (data.E <= 0 || data.R < 0 || data.F < 0 || data.S < 0)
//     {
//         throw runtime_error("Invalid header values.");
//     }

//     /*
//      * Racuna minimalan broj vrijednosti potreban za obavezne dijelove datoteke:
//      *
//      * 4        - vrijednosti zaglavlja
//      * R        - kapaciteti prostorija
//      * S * E    - matrica studentEvent
//      * R * F    - matrica roomFeature
//      * E * F    - matrica eventFeature
//      *
//      * Matrica eventTimeslot i opcionalna matrica precedence ovdje se još ne
//      * racunaju jer se broj termina naknadno zakljucuje iz preostalih podataka.
//      */
//     size_t needed = 4 + data.R + size_t(data.S) * data.E + size_t(data.R) * data.F + size_t(data.E) * data.F;

//     // Provjerava postoji li dovoljno brojeva za sve obavezne dijelove.
//     if (values.size() < needed)
//     {
//         throw runtime_error("Input file does not contain expected number of mandatory values.");
//     }

//     // Ucitava kapacitete prostorija.
//     // roomSizes[i] predstavlja kapacitet prostorije r.
//     data.roomSizes.resize(data.R);
//     for (int i = 0; i < data.R; i++)
//     {
//         data.roomSizes[i] = static_cast<int>(values[idx++]);
//     }

//     /*
//      * Ucitava matricu studentEvent dimenzija S x E.
//      * studentEvent[s][e] najcešce ima vrijednost:
//      * 1 - student s sudjeluje na dogadaju e
//      * 0 - student s ne sudjeluje na dogadaju e
//      */
//     data.studentEvent.resize(data.S, vector<int>(data.E));
//     for (int s = 0; s < data.S; s++)
//     {
//         for (int e = 0; e < data.E; e++)
//         {
//             data.studentEvent[s][e] = static_cast<int>(values[idx++]);
//         }
//     }

//     //Ucitava matricu roomFeature dimenzija R x F.
//     //roomFeature[r][f] oznacava posjeduje li prostorija r znacajku f.
//     data.roomFeature.resize(data.R, vector<int>(data.F));
//     for (int r = 0; r < data.R; r++)
//     {
//         for (int f = 0; f < data.F; f++)
//         {
//             data.roomFeature[r][f] = static_cast<int>(values[idx++]);
//         }
//     }

//     // Ucitava matricu eventFeature dimenzija E x F.
//     // eventFeature[e][f] oznacava zahtijeva li dogadaj e znacajku f.
//     data.eventFeature.resize(data.E, vector<int>(data.F));
//     for (int e = 0; e < data.E; e++)
//     {
//         for (int f = 0; f < data.F; f++)
//         {
//             data.eventFeature[e][f] = static_cast<int>(values[idx++]);
//         }
//     }

//     // Broj vrijednosti koje su ostale nakon svih obaveznih dijelova.
//     size_t remaining = values.size() - idx;
//     // Za matricu eventTimeslot potreban je barem jedan podatak za svaki dogadaj, najmanje E preostalih vrijednosti
//     if (remaining < static_cast<size_t>(data.E))
//     {
//         throw runtime_error("Not enough remaining values for event-timeslot section.");
//     }

//     // Ako postoji matrica prethodnosti, ona je dimenzija E x E, pa zauzima
//     // E * E vrijednosti.
//     size_t matrixEE = size_t(data.E) * data.E;

//     // Broj termina T nije zapisan u zaglavlju pa ga parser mora zakljuciti.
//     int inferredT = -1;
//     bool hasPrecedence = false;

//     // Parser prvo provjerava može li se od preostalih podataka odvojiti E x E
//     // matrica prethodnosti tako da ostatak bude djeljiv s E.
//     if (remaining >= matrixEE && (remaining - matrixEE) % data.E == 0)
//     {
//         // remaining = E * T + E * E
//         // T = (remaining - E * E) / E
//         hasPrecedence = true;
//         inferredT = static_cast<int>((remaining - matrixEE) / data.E);
//     }
//     else
//     {
//         // remaining = E * T
//         // T = remaining / E
//         inferredT = static_cast<int>(remaining / data.E);
//         hasPrecedence = false;
//     }

//     // Broj termina mora biti pozitivan.
//     if (inferredT <= 0)
//     {
//         throw runtime_error("Inferred number of timeslots is non-positive.");
//     }

//     // Ucitava matricu eventTimeslot dimenzija E x T.
//     // eventTimeslot[e][t] oznacava smije li se dogadaj e održati u terminu t.
//     data.eventTimeslot.resize(data.E, vector<int>(inferredT));
//     for (int e = 0; e < data.E; e++)
//     {
//         for (int t = 0; t < inferredT; t++)
//         {
//             if (idx >= values.size()) // Dodatna zaštita od pokušaja citanja izvan vektora values.
//                 throw runtime_error("Ran out of numbers while reading event-timeslot matrix.");
//             data.eventTimeslot[e][t] = static_cast<int>(values[idx++]);
//         }
//     }

//     // Ako je prepoznata matrica prethodnosti, ucitava se E x E vrijednosti.
//     // precedence[i][j] oznacava postoji li zahtjev da dogadaj i prethodi dogadaju j.
//     if (hasPrecedence)
//     {
//         data.precedence.resize(data.E, vector<int>(data.E));
//         for (int i = 0; i < data.E; i++)
//         {
//             for (int j = 0; j < data.E; j++)
//             {
//                 if (idx >= values.size()) // Dodatna zaštita od pokušaja citanja izvan vektora values.
//                     throw runtime_error("Ran out of numbers while reading precedence matrix.");
//                 data.precedence[i][j] = static_cast<int>(values[idx++]);
//             }
//         }
//     }
//     else
//     {
//         data.precedence.clear();
//     }

//     return data;
// }

// int TimData::numberof_timeslots(int i){
//     int k = 0;
//     for(auto& x : eventTimeslot[i])
//         k += x;
//     return k;
// }