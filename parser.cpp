#include <sstream>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include "parser.h"
using namespace std;

/*
 * Učitava podatke iz .tim datoteke i sprema ih u strukturu TimData.
 *
 * Očekivani redoslijed podataka u datoteci je:
 *  1. E, R, F, S
 *  2. kapaciteti prostorija
 *  3. matrica student–događaj
 *  4. matrica prostorija–značajka
 *  5. matrica događaj–značajka
 *  6. matrica događaj–termin
 *  7. opcionalna matrica prethodnosti događaja
 */
TimData loadTIM(const string &filename)
{
    // Otvara ulaznu datoteku čiji je naziv ili put predan funkciji.
    ifstream file(filename);

    // Ako datoteku nije moguće otvoriti, prekida učitavanje iznimkom.
    if (!file.is_open())
    {
        throw runtime_error("Ne mogu otvoriti datoteku.");
    }

    
    // Učitava cijeli sadržaj datoteke u jedan string.
    // Stringstream privremeno prima sadržaj datoteke preko file.rdbuf().
    string text;
    {
        stringstream ss;
        ss << file.rdbuf();
        text = ss.str();
    }

    /*
     * Iz teksta izdvaja samo tokene koji u cijelosti predstavljaju cijele brojeve.
     *
     * v sadrži sve pronađene brojeve redom kojim se pojavljuju u datoteci.
     * long long se koristi tijekom učitavanja kako bi se smanjio rizik od
     * prekoračenja prije pretvorbe u int.
     */
    vector<long long> values;
    string token;
    stringstream ss(text);
    while (ss >> token)
    {
        size_t pos;
        // stoll pretvara početni brojčani dio tokena u long long.
        long long v = stoll(token, &pos);
        // Token se prihvaća samo ako je cijeli token uspješno pretvoren u broj.
        if (pos == token.size())
            values.push_back(v);
    }

    /*
     * Prva četiri broja moraju postojati jer predstavljaju zaglavlje:
     * E - broj događaja
     * R - broj prostorija
     * F - broj značajki prostorija i događaja
     * S - broj studenata
     */
    if (values.size() < 4)
    {
        throw runtime_error("Input file is too short after filtering tokens.");
    }

    // idx označava poziciju sljedećeg broja koji treba pročitati iz values.
    size_t idx = 0;

    // Struktura u koju se spremaju svi učitani podaci.
    TimData data;
    // Čitanje četiriju vrijednosti zaglavlja.
    data.E = static_cast<int>(values[idx++]);
    data.R = static_cast<int>(values[idx++]);
    data.F = static_cast<int>(values[idx++]);
    data.S = static_cast<int>(values[idx++]);


    // Provjerava osnovnu ispravnost zaglavlja.
    // Broj događaja mora biti pozitivan, dok ostale vrijednosti ne smiju biti negativne.
    if (data.E <= 0 || data.R < 0 || data.F < 0 || data.S < 0)
    {
        throw runtime_error("Invalid header values.");
    }

    /*
     * Računa minimalan broj vrijednosti potreban za obavezne dijelove datoteke:
     *
     * 4        - vrijednosti zaglavlja
     * R        - kapaciteti prostorija
     * S * E    - matrica studentEvent
     * R * F    - matrica roomFeature
     * E * F    - matrica eventFeature
     *
     * Matrica eventTimeslot i opcionalna matrica precedence ovdje se još ne
     * računaju jer se broj termina naknadno zaključuje iz preostalih podataka.
     */
    size_t needed = 4 + data.R + size_t(data.S) * data.E + size_t(data.R) * data.F + size_t(data.E) * data.F;

    // Provjerava postoji li dovoljno brojeva za sve obavezne dijelove.
    if (values.size() < needed)
    {
        throw runtime_error("Input file does not contain expected number of mandatory values.");
    }

    // Učitava kapacitete prostorija.
    // roomSizes[i] predstavlja kapacitet prostorije r.
    data.roomSizes.resize(data.R);
    for (int i = 0; i < data.R; i++)
    {
        data.roomSizes[i] = static_cast<int>(values[idx++]);
    }

    /*
     * Učitava matricu studentEvent dimenzija S x E.
     * studentEvent[s][e] najčešće ima vrijednost:
     * 1 - student s sudjeluje na događaju e
     * 0 - student s ne sudjeluje na događaju e
     */
    data.studentEvent.resize(data.S, vector<int>(data.E));
    for (int s = 0; s < data.S; s++)
    {
        for (int e = 0; e < data.E; e++)
        {
            data.studentEvent[s][e] = static_cast<int>(values[idx++]);
        }
    }

    //Učitava matricu roomFeature dimenzija R x F.
    //roomFeature[r][f] označava posjeduje li prostorija r značajku f.
    data.roomFeature.resize(data.R, vector<int>(data.F));
    for (int r = 0; r < data.R; r++)
    {
        for (int f = 0; f < data.F; f++)
        {
            data.roomFeature[r][f] = static_cast<int>(values[idx++]);
        }
    }

    // Učitava matricu eventFeature dimenzija E x F.
    // eventFeature[e][f] označava zahtijeva li događaj e značajku f.
    data.eventFeature.resize(data.E, vector<int>(data.F));
    for (int e = 0; e < data.E; e++)
    {
        for (int f = 0; f < data.F; f++)
        {
            data.eventFeature[e][f] = static_cast<int>(values[idx++]);
        }
    }

    // Broj vrijednosti koje su ostale nakon svih obaveznih dijelova.
    size_t remaining = values.size() - idx;
    // Za matricu eventTimeslot potreban je barem jedan podatak za svaki događaj, najmanje E preostalih vrijednosti
    if (remaining < static_cast<size_t>(data.E))
    {
        throw runtime_error("Not enough remaining values for event-timeslot section.");
    }

    // Ako postoji matrica prethodnosti, ona je dimenzija E x E, pa zauzima
    // E * E vrijednosti.
    size_t matrixEE = size_t(data.E) * data.E;

    // Broj termina T nije zapisan u zaglavlju pa ga parser mora zaključiti.
    int inferredT = -1;
    bool hasPrecedence = false;

    // Parser prvo provjerava može li se od preostalih podataka odvojiti E x E
    // matrica prethodnosti tako da ostatak bude djeljiv s E.
    if (remaining >= matrixEE && (remaining - matrixEE) % data.E == 0)
    {
        // remaining = E * T + E * E
        // T = (remaining - E * E) / E
        hasPrecedence = true;
        inferredT = static_cast<int>((remaining - matrixEE) / data.E);
    }
    else
    {
        // remaining = E * T
        // T = remaining / E
        inferredT = static_cast<int>(remaining / data.E);
        hasPrecedence = false;
    }

    // Broj termina mora biti pozitivan.
    if (inferredT <= 0)
    {
        throw runtime_error("Inferred number of timeslots is non-positive.");
    }

    // Učitava matricu eventTimeslot dimenzija E x T.
    // eventTimeslot[e][t] označava smije li se događaj e održati u terminu t.
    data.eventTimeslot.resize(data.E, vector<int>(inferredT));
    for (int e = 0; e < data.E; e++)
    {
        for (int t = 0; t < inferredT; t++)
        {
            if (idx >= values.size()) // Dodatna zaštita od pokušaja čitanja izvan vektora values.
                throw runtime_error("Ran out of numbers while reading event-timeslot matrix.");
            data.eventTimeslot[e][t] = static_cast<int>(values[idx++]);
        }
    }

    // Ako je prepoznata matrica prethodnosti, učitava se E x E vrijednosti.
    // precedence[i][j] označava postoji li zahtjev da događaj i prethodi događaju j.
    if (hasPrecedence)
    {
        data.precedence.resize(data.E, vector<int>(data.E));
        for (int i = 0; i < data.E; i++)
        {
            for (int j = 0; j < data.E; j++)
            {
                if (idx >= values.size()) // Dodatna zaštita od pokušaja čitanja izvan vektora values.
                    throw runtime_error("Ran out of numbers while reading precedence matrix.");
                data.precedence[i][j] = static_cast<int>(values[idx++]);
            }
        }
    }
    else
    {
        data.precedence.clear();
    }

    return data;
}

int TimData::numberof_timeslots(int i){
    int k = 0;
    for(auto& x : eventTimeslot[i])
        k += x;
    return k;
}