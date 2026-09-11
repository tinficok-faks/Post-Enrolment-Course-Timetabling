#include "tabu_search.h"

#include <algorithm>
#include <iostream>

// konstruktor

TabuSearch::TabuSearch(
    std::vector<int>& roomSizes,
    std::vector<std::vector<int>>& placedEvents,
    std::vector<int>& unplacedEvents,
    const std::vector<std::vector<int>>& conflictList,
    const std::vector<std::vector<int>>& studentsOfEvent,
    const std::vector<std::vector<int>>& roomFeature,
    const std::vector<std::vector<int>>& eventFeature,
    const std::vector<std::vector<int>>& eventTimeslot,
    const std::vector<std::vector<int>>& precedence,
    int S
)
    : roomSizes(roomSizes),
      placedEvents(placedEvents),
      unplacedEvents(unplacedEvents),
      conflictList(conflictList),
      studentsOfEvent(studentsOfEvent),
      roomFeature(roomFeature),
      eventFeature(eventFeature),
      eventTimeslot(eventTimeslot),
      precedence(precedence),
      S(S),
      rng(std::random_device{}())
{
    numberOfEvents =
        static_cast<int>(studentsOfEvent.size());

    numberOfTimeslots =
        static_cast<int>(placedEvents.size());

    numberOfRooms =
        static_cast<int>(roomSizes.size());

    initializePositions();


    // tabuUntil[event][timeslot]
    tabuUntil.assign(
        numberOfEvents,
        std::vector<int>(numberOfTimeslots, 0)
    );


    currentIteration = 0;

    bestCost = static_cast<int>(unplacedEvents.size());
}



// inicijalne pozicije evenata

void TabuSearch::initializePositions(){

    // za svaki event spremamo gdje se trenutno nalazi
    currentTimeslot.assign(numberOfEvents, -1);
    currentRoom.assign(numberOfEvents, -1);

    for (int timeslot = 0; timeslot < numberOfTimeslots; ++timeslot){
        for (int room = 0; room < numberOfRooms; ++room){
            int event = placedEvents[timeslot][room];

            if (event == -1)
                continue;


            currentTimeslot[event] = timeslot;
            currentRoom[event] = room;
        }
    }
}


// provjerava je li event u vektoru

bool TabuSearch::contains(const std::vector<int>& v, int value){
    return std::find(v.begin(), v.end(), value) != v.end();
}



// provjerava odgovara li soba eventu

bool TabuSearch::roomCompatible(int event, int room){

    if (roomSizes[room] < static_cast<int>(studentsOfEvent[event].size())){
        return false;
    }

    // feature sobe

    for (int feature = 0;
         feature < static_cast<int>(eventFeature[event].size());
         ++feature)
    {
        // event zahtijeva feature,
        // ali soba ga nema
        if (eventFeature[event][feature] == 1 &&
            roomFeature[room][feature] == 0)
        {
            return false;
        }
    }


    return true;
}


// pronalazak konflikta

std::vector<int> TabuSearch::collectConflicts(int event, int timeslot){

    std::vector<int> conflicts;

    // lambda funkcija
    auto addConflict = [&](int otherEvent){

            if (otherEvent < 0)
                return;

            if (!contains(conflicts, otherEvent))
                conflicts.push_back(otherEvent);
        };


    // 1. konflikti studenata

    // conflictList[event] vec sadrzi sve evente
    // koji dijele barem jednog studenta s eventom
    for (int otherEvent : conflictList[event])
    {
        // problem postoji samo ako je drugi event
        // u timeslotu u koji zelimo staviti event
        if (currentTimeslot[otherEvent] == timeslot){
            addConflict(otherEvent);
        }
    }
    
    // 2. precedence uvjet

    for (int otherEvent = 0; otherEvent < numberOfEvents; ++otherEvent){
        
        // nerasporedeni event trenutno ne moze
        // stvarati precedence konflikt
        if (currentTimeslot[otherEvent] == -1)
            continue;

        // otherEvent mora biti PRIJE eventa
        // precedence[otherEvent][event] == 1

        if (precedence[otherEvent][event] == 1)
        {
            // ako je otherEvent u istom ili kasnijem
            // terminu, precedence bi bio prekrsen
            if (currentTimeslot[otherEvent] >= timeslot){
                addConflict(otherEvent);
            }
        }


        // event mora biti PRIJE otherEventa
        // precedence[event][otherEvent] == 1

        if (precedence[event][otherEvent] == 1)
        {
            // ako je otherEvent u istom ili ranijem
            // terminu, precedence bi bio prekrsen
            if (currentTimeslot[otherEvent] <= timeslot){
                addConflict(otherEvent);
            }
        }
    }


    return conflicts;
}


// pronalazak najboljeg TABU SEARCH poteza

Move TabuSearch::findBestMove() {
    Move bestMove;

    int minimumCost = 1e9;

    // pregledaj sve nerasporedene evente
    for (int event : unplacedEvents){

        // pregledaj sve timeslotove

        for (int timeslot = 0; timeslot < numberOfTimeslots; ++timeslot){

            // event se u ovom terminu uopce
            // ne smije odrzavati
            if (eventTimeslot[event][timeslot] == 0){
                continue;
            }


            // ukupni konflikti
            std::vector<int> baseConflicts = collectConflicts(event, timeslot);


            // pregledaj sve sobe
            for (int room = 0; room < numberOfRooms; ++room){

                if (!roomCompatible(event, room)){
                    continue;
                }


                std::vector<int> ejectedEvents = baseConflicts;


                // room constraint
                //
                // ako netko vec koristi tu sobu u tom terminu,
                // mora biti izbacen

                int occupant = placedEvents[timeslot][room];


                if (occupant != -1 && !contains(ejectedEvents, occupant)){
                    ejectedEvents.push_back(occupant);
                }


                // COST FUNKCIJA
                //
                // trenutno nerasporedeni eventi:
                // |unplacedEvents|
                //
                // event koji ubacujemo nestaje iz unplacedEvents
                // pa |unplacedEvents| - 1
                //
                // svi izbaceni eventi ulaze u unplaced:
                // + |ejectedEvents|
                //
                // =>
                // f(candidate) =
                // |unplaced| - 1 + |ejected|
                //

                int candidateCost =
                    static_cast<int>(unplacedEvents.size()) - 1
                    + static_cast<int>(ejectedEvents.size());


                // TABU provjera
                // zabrana vracanja izbacenih evenata na staro mjesto

                bool isTabu = tabuUntil[event][timeslot] > currentIteration;


                // Aspiration:
                //
                // ako tabu potez daje bolje rjesenje od
                // najboljeg ikad pronadenog, dopustamo ga
                bool aspiration = candidateCost < bestCost;


                if (isTabu && !aspiration)
                    continue;


                // BEST ADMISSIBLE MOVE

                if (candidateCost < minimumCost)
                {
                    minimumCost = candidateCost;
                    bestMove.event = event;
                    bestMove.timeslot = timeslot;
                    bestMove.room = room;
                    bestMove.ejectedEvents = ejectedEvents;
                    bestMove.cost = candidateCost;
                }
            }
        }
    }


    return bestMove;
}


// primjena odabranog poteza

void TabuSearch::applyMove(const Move& move){

    // spremamo staru poziciju svakog eventa kojeg
    // izbacujemo
    // potrebna nam je zbog tabu zabrane:
    std::vector<std::pair<int, int>> oldPositions;

    // 1. izbaci konfliktne evente

    for (int event : move.ejectedEvents){
        int oldTimeslot = currentTimeslot[event];

        int oldRoom = currentRoom[event];

        // ako se dogodi da ejectedEvent nije 
        // bio rasporedeni event
        if (oldTimeslot == -1 || oldRoom == -1)
            continue;

        // makni event iz matrice
        placedEvents[oldTimeslot][oldRoom] = -1;


        // zapamti stari timeslot
        oldPositions.push_back({ event, oldTimeslot });


        // oznaci event kao nerasporeden
        currentTimeslot[event] = -1;
        currentRoom[event] = -1;


        // dodaj ga u listu nerasporedenih
        //
        // provjera sprjecava eventualne duplikate
        if (!contains(unplacedEvents, event)){
            unplacedEvents.push_back(event);
        }
    }
    

    // 2. maknimo novi event iz unplacedEvents

    auto position =
        std::find(
            unplacedEvents.begin(),
            unplacedEvents.end(),
            move.event
        );


    if (position != unplacedEvents.end())
        unplacedEvents.erase(position);


    // 3. ubaci event u novu poziciju

    placedEvents[move.timeslot][move.room] = move.event;

    currentTimeslot[move.event] = move.timeslot;


    currentRoom[move.event] = move.room;


    // 4. TABU TENURE
    //
    // sadasnja vrijednost:
    // random[0,10) + |unplacedE|
    //
    // odnosno random vrijednost 0..9
    // + trenutni broj nerasporedenih eventa
    //

    std::uniform_int_distribution<int> randomTenure(0, 9);


    int tenure = randomTenure(rng) +
        static_cast<int>(unplacedEvents.size());


    // eventi koje smo izbacili ne smiju se
    // odmah vratiti u stari timeslot
    for (const auto& position : oldPositions){
        int event = position.first;
        int oldTimeslot = position.second;

        tabuUntil[event][oldTimeslot] = currentIteration + tenure;
    }


    // 5. update BEST COST

    int currentCost = static_cast<int>(unplacedEvents.size());


    if (currentCost < bestCost)
        bestCost = currentCost;
}


// pocetni TABU SEARCH

bool TabuSearch::solve(int maxIterations){

    currentIteration = 0;

    // spremamo najbolje rjesenje


    std::vector<std::vector<int>> bestPlacedEvents = placedEvents;

    std::vector<int> bestUnplacedEvents = unplacedEvents;

    int locallyBestCost = static_cast<int>(unplacedEvents.size());

    bestCost = locallyBestCost;

    std::cout
        << "Tabu Search pocinje s "
        << unplacedEvents.size()
        << " nerasporedenih eventa."
        << std::endl;


    if (unplacedEvents.empty())
        return true;

    // glavna petlja

    while (!unplacedEvents.empty() && currentIteration < maxIterations){
        
        ++currentIteration;

        // pronadi najbolji admissible susjed

        Move bestMove = findBestMove();

        // ako ne postoji potez, mozda je tabu lista
        // sve blokirala, ako da resetiramo tabu listu

        if (!bestMove.exists()){
            for (int event = 0; event < numberOfEvents; ++event){
                std::fill(
                    tabuUntil[event].begin(),
                    tabuUntil[event].end(),
                    0
                );
            }


            bestMove = findBestMove();

            // ako ni bez tabu zabrane ne postoji potez,
            // ne mozemo nastaviti
            if (!bestMove.exists())
            {
                std::cout
                    << "Nije pronaden niti jedan dopusten potez."
                    << std::endl;
                break;
            }
        }


        // primijeni najbolji potez

        applyMove(bestMove);


        // spremi najbolje rjesenje koje si nasao
    }


    // FEASIBLE RJESENJE

    if (unplacedEvents.empty()){
        std::cout
            << "Pronaden feasible raspored nakon "
            << currentIteration
            << " iteracija."
            << std::endl;

        return true;
    }


    // NIJE PRONADEN FEASIBLE
    //
    // ne zelimo ostaviti zadnje trenutno rjesenje ako je losije
    // od najboljega koje smo vidjeli
    //
    // Zato vracamo najbolje rjesenje

    placedEvents = bestPlacedEvents;
    unplacedEvents = bestUnplacedEvents;

    // ponovno izgradi currentTimeslot/currentRoom jer smo
    // upravo vratili starije stanje rasporeda
    initializePositions();

    bestCost = locallyBestCost;

    std::cout
        << "Tabu Search nije pronasao feasible raspored unutar "
        << maxIterations
        << " iteracija."
        << std::endl;

    std::cout
        << "Najbolji raspored ima "
        << locallyBestCost
        << " nerasporedenih eventa."
        << std::endl;


    return false;
}