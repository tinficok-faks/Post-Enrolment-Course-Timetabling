
#include "local_search.h"
#include <iostream>

int dailyPenalty(int student, int day, const std::vector<std::vector<int>>& studentSchedule){

    const int dailySlots = 9;
    const int firstSlot = day * dailySlots;

    int penalty = 0;
    int Eventnumber = 0;
    int consecutive = 0;

    // svi timeslotovi toga dana
    for(int i = 0; i < dailySlots; ++i){
        if(studentSchedule[student][firstSlot + i] == 1){
            Eventnumber++;
            consecutive++;
            if(i == 8 && (consecutive > 2)){
                penalty += consecutive - 2;
            }
        }
        else{
            // SC1: 3+ uzastopna eventa u danu
            if(consecutive > 2){
                penalty += consecutive - 2;
            }
            consecutive = 0;
        }
    }

    // SC1: samo 1 event u danu
    if (Eventnumber == 1) 
        penalty++;

    // SC1: zadnji timeslot u danu
    if(studentSchedule[student][firstSlot + 8] == 1)
        penalty++;

    return penalty;
}

int getCostChange(int event, int oldts, int newts, 
    const std::vector<std::vector<int>>& studentsOfEvent,
    std::vector<std::vector<int>>& studentSchedule
){
    int oldDay = oldts / 9;
    int newDay = newts / 9;

    int before = 0;
    int after = 0;

    // racunamo povredu mekih uvjeta po svakom studentu
    for(auto& student : studentsOfEvent[event]){

        before += dailyPenalty(student, oldDay, studentSchedule);

        // gledamo promjenu u oba dana
        if (newDay != oldDay) {
            before += dailyPenalty(student, newDay, studentSchedule);
        }

        studentSchedule[student][oldts] = 0;
        studentSchedule[student][newts] = 1;


        after += dailyPenalty(student, oldDay, studentSchedule);

        if (newDay != oldDay) {
            after += dailyPenalty(student, newDay, studentSchedule);
        }

        studentSchedule[student][oldts] = 1;
        studentSchedule[student][newts] = 0;
    }

    // na kraju gledamo njihovu razliku
    return after - before;
}


// pomocna matrica za brzo pretrazivanje
std::vector<std::vector<int>> buildStudentSchedule(
    int S, // broj studenata
    const std::vector<std::vector<int>>& placedEvents,
    const std::vector<std::vector<int>>& studentsOfEvent
) {
    int numberOfTimeslots = 45;

    int numberOfRooms = static_cast<int>(placedEvents[0].size());

    std::vector<std::vector<int>> StudentSchedule(
        S,
        std::vector<int>(numberOfTimeslots, 0)
    );

    for (int ts = 0; ts < numberOfTimeslots; ++ts){
        for (int room = 0; room < numberOfRooms; ++room){
            int event = placedEvents[ts][room];

            if (event == -1)
                continue;

            for (int student : studentsOfEvent[event])
                StudentSchedule[student][ts] = 1;

        }
    }

    return StudentSchedule;
}

// prebacujemo event u drugi slobodni timeslot
void findBestTransfer(
    TabuSearch& schedule, 
    Move& bestMove,
    int& bestDelta,
    std::vector<std::vector<int>>& StudentSchedule
){
    for (int event = 0; event < schedule.numberOfEvents; ++event) {

        for (int timeslot = 0; timeslot < schedule.numberOfTimeslots; ++timeslot) {

            if (timeslot == schedule.currentTimeslot[event])
               continue;

            // je li timeslot dopusten
            if (schedule.eventTimeslot[event][timeslot] == 0)
               continue;


            // ima li konflikata
            bool conflict_exists = false;

            for(int conflict : schedule.conflictList[event])
               if(schedule.currentTimeslot[conflict] == timeslot){
                   conflict_exists = true;
                   break;
               }
            
            if (conflict_exists)
                 continue;

            // je li precedence zadovoljen
            for (int otherEvent = 0; otherEvent < schedule.numberOfEvents; ++otherEvent){

                if (otherEvent == event)
                   continue;
                // nerasporedeni event trenutno ne moze
                // stvarati precedence konflikt.
                if (schedule.currentTimeslot[otherEvent] == -1)
                    continue;
             
                // otherEvent mora biti PRIJE eventa
                // precedence[otherEvent][event] == 1
                if (schedule.precedence[otherEvent][event] == 1){
                    // ako je otherEvent u istom ili kasnijem
                    // terminu, precedence bi bio prekrsen
                    if (schedule.currentTimeslot[otherEvent] >= timeslot){
                        conflict_exists = true;
                        break;
                    }
                }
             
             
                // event mora biti PRIJE otherEventa
                // precedence[event][otherEvent] == 1
             
                if (schedule.precedence[event][otherEvent] == 1){
                    // ako je otherEvent u istom ili ranijem
                    // terminu, precedence bi bio prekrsen
                    if (schedule.currentTimeslot[otherEvent] <= timeslot){
                        conflict_exists = true;
                        break;
                    }
                }
             }
             
            if (conflict_exists)
                    continue;
             
            int room = -1;         

            // postoji li kompatibilna soba
            for(int i = 0; i < schedule.numberOfRooms; ++i){
                if(schedule.roomCompatible(event, i) 
                   && schedule.placedEvents[timeslot][i] == -1){
                    room = i;
                    break;
                }
            }
         
            if (room == -1)
                continue;

         
            int delta = getCostChange(event, 
                                      schedule.currentTimeslot[event],
                                      timeslot,
                                      schedule.studentsOfEvent,
                                      StudentSchedule);

         
            if (delta < bestDelta) {
                bestDelta = delta;
            
                bestMove.event = event;
                bestMove.timeslot = timeslot;
                bestMove.room = room;
                bestMove.method = 't';
            }
        }
    }
}

int getSwapCostChange(
    int S,
    int event1,
    int event2,
    int timeslot1,
    int timeslot2,
    const std::vector<std::vector<int>>& studentsOfEvent,
    std::vector<std::vector<int>>& studentSchedule
) {
    const int numberOfStudents = S;

    const int day1 = timeslot1 / 9;
    const int day2 = timeslot2 / 9;

    std::vector<bool> affected(numberOfStudents, false);
    std::vector<bool> attendsEvent1(numberOfStudents, false);
    std::vector<bool> attendsEvent2(numberOfStudents, false);

    for (int student : studentsOfEvent[event1]) {
        affected[student] = true;
        attendsEvent1[student] = true;
    }

    for (int student : studentsOfEvent[event2]) {
        affected[student] = true;
        attendsEvent2[student] = true;
    }

    int before = 0;
    int after = 0;

    for (int student = 0; student < numberOfStudents; ++student){

        if (!affected[student])
            continue;

        // cost prije swapa

        before += dailyPenalty(student, day1, studentSchedule);

        if (day1 != day2) {
            before += dailyPenalty(student, day2, studentSchedule);
        }

        // privremeno napravi swap

        bool inEvent1 = attendsEvent1[student];
        bool inEvent2 = attendsEvent2[student];

        // student pohada samo event1:
        // event1 ide iz timeslot1 u timeslot2
        if (inEvent1 && !inEvent2) {
            studentSchedule[student][timeslot1] = 0;
            studentSchedule[student][timeslot2] = 1;
        }

        // student pohada samo event2:
        // event2 ide iz timeslot2 u timeslot1
        else if (!inEvent1 && inEvent2) {
            studentSchedule[student][timeslot2] = 0;
            studentSchedule[student][timeslot1] = 1;
        }

        // ako student pohada oba eventa,
        // i prije i poslije ima predavanje u oba timeslota,
        // pa se njegov raspored ne mijenja


        // cost nakon swapa+

        after += dailyPenalty(student, day1, studentSchedule);

        if (day1 != day2) {
            after += dailyPenalty(student, day2, studentSchedule);
        }


        // vrati na staro

        if (inEvent1 && !inEvent2) {
            studentSchedule[student][timeslot1] = 1;
            studentSchedule[student][timeslot2] = 0;
        }
        else if (!inEvent1 && inEvent2) {
            studentSchedule[student][timeslot2] = 1;
            studentSchedule[student][timeslot1] = 0;
        }
    }

    return after - before;
}

// zamjenjujemo timeslotove 2 eventa
void findBestSwap(
    TabuSearch& schedule,
    Move& bestMove,
    int& bestDelta,
    std::vector<std::vector<int>>& studentSchedule
) {
    for (int event1 = 0; event1 < schedule.numberOfEvents; ++event1){

        int timeslot1 = schedule.currentTimeslot[event1];


        // event2 pocinje od event1 + 1 jer nema potrebe
        // provjeravati isti par dvaput.
        for (int event2 = event1 + 1;
             event2 < schedule.numberOfEvents;
             ++event2)
        {
            int timeslot2 = schedule.currentTimeslot[event2];

            int room2 = schedule.currentRoom[event2];

            if (timeslot2 == -1 || room2 == -1)
                continue;


            // ako su vec u istom timeslotu, zamjena soba
            // ne mijenja soft cost
            if (timeslot1 == timeslot2)
                continue;


            // 1. availability

            // event1 ide u timeslot2
            if (schedule.eventTimeslot[event1][timeslot2] == 0)
                continue;

            // event2 ide u timeslot1
            if (schedule.eventTimeslot[event2][timeslot1] == 0)
                continue;


            // 2. room constraints

            int newRoom1 = -1;
            int newRoom2 = -1;        

            // postoji li kompatibilna soba za event1
            for(int i = 0; i < schedule.numberOfRooms; ++i){
                int occupant = schedule.placedEvents[timeslot2][i];
                if(schedule.roomCompatible(event1, i) 
                   && (occupant == -1 || occupant == event2)){
                    newRoom1 = i;
                    break;
                }
            }

            // postoji li kompatibilna soba za event2
            for(int i = 0; i < schedule.numberOfRooms; ++i){
                int occupant = schedule.placedEvents[timeslot1][i];
                if(schedule.roomCompatible(event2, i) 
                   && (occupant == -1 || occupant == event1)){
                    newRoom2 = i;
                    break;
                }
            }

            if (newRoom1 == -1 || newRoom2 == -1)
                continue;

            // 3. konflikti zbog studenata

            bool conflict_exists = false;


            // event1 se seli u timeslot2
            for (int conflict : schedule.conflictList[event1])
            {
                // event2 napusta timeslot2, pa njega
                // ne gledamo kao konflikt
                if (conflict == event2)
                    continue;

                if (schedule.currentTimeslot[conflict]
                    == timeslot2)
                {
                    conflict_exists = true;
                    break;
                }
            }

            if (conflict_exists)
                continue;


            // event2 se seli u timeslot1
            for (int conflict : schedule.conflictList[event2])
            {
                // event1 napusta timeslot1
                if (conflict == event1)
                    continue;

                if (schedule.currentTimeslot[conflict] == timeslot1){
                    conflict_exists = true;
                    break;
                }
            }

            if (conflict_exists)
                continue;


            // 4. precedence

            // pomocna lambda funckija:
            // vraca timeslot eventa nakon potencijalnog swapa
            auto projectedTimeslot =
                [&](int event)
                {
                    if (event == event1)
                        return timeslot2;

                    if (event == event2)
                        return timeslot1;

                    return schedule.currentTimeslot[event];
                };


            for (int otherEvent = 0; otherEvent < schedule.numberOfEvents; ++otherEvent){

                int otherTimeslot = projectedTimeslot(otherEvent);

                if (otherTimeslot == -1)
                    continue;


                // precedence za event1

                // event1 mora biti prije otherEvent
                if (schedule.precedence[event1][otherEvent] == 1)
                {
                    if (projectedTimeslot(event1) >= otherTimeslot)
                    {
                        conflict_exists = true;
                        break;
                    }
                }

                // otherEvent mora biti prije event1
                if (schedule.precedence[otherEvent][event1] == 1)
                {
                    if (otherTimeslot >= projectedTimeslot(event1))
                    {
                        conflict_exists = true;
                        break;
                    }
                }


                // precedence za event2

                // event2 mora biti prije otherEvent
                if (schedule.precedence[event2][otherEvent] == 1)
                {
                    if (projectedTimeslot(event2) >= otherTimeslot)
                    {
                        conflict_exists = true;
                        break;
                    }
                }

                // otherEvent mora biti prije event2
                if (schedule.precedence[otherEvent][event2] == 1)
                {
                    if (otherTimeslot
                        >= projectedTimeslot(event2))
                    {
                        conflict_exists = true;
                        break;
                    }
                }
            }


            if (conflict_exists)
                continue;


            // 5. cost change

            int delta =
                getSwapCostChange(
                    schedule.S,
                    event1,
                    event2,
                    timeslot1,
                    timeslot2,
                    schedule.studentsOfEvent,
                    studentSchedule
                );


            // 6. best delta

            if (delta < bestDelta){
                bestDelta = delta;

                bestMove.event = event1;
                bestMove.secondEvent = event2;

                bestMove.timeslot = timeslot2;
                bestMove.room = newRoom1;
                bestMove.room2 = newRoom2;
                bestMove.method = 's';
            }
        }
    }
}

void applyMove(
    TabuSearch& schedule, 
    const Move& move,
    std::vector<std::vector<int>>& studentSchedule
){

    const int oldTimeslot = schedule.currentTimeslot[move.event];
    const int oldRoom = schedule.currentRoom[move.event];

    if (oldTimeslot < 0 || oldRoom < 0) {
        return;
    }

    schedule.placedEvents[oldTimeslot][oldRoom] = -1;
    schedule.placedEvents[move.timeslot][move.room] = move.event;
    schedule.currentTimeslot[move.event] = move.timeslot;
    schedule.currentRoom[move.event] = move.room;

    for (int student : schedule.studentsOfEvent[move.event]) {
        studentSchedule[student][oldTimeslot] = 0;
        studentSchedule[student][move.timeslot] = 1;
    }
}

void applySwap(
    TabuSearch& schedule,
    const Move& move,
    std::vector<std::vector<int>>& studentSchedule
) {
    int event1 = move.event;
    int event2 = move.secondEvent;

    // stare pozicije
    int timeslot1 = schedule.currentTimeslot[event1];
    int room1 = schedule.currentRoom[event1];

    int timeslot2 = schedule.currentTimeslot[event2];
    int room2 = schedule.currentRoom[event2];

    if (timeslot1 < 0 || room1 < 0 ||
        timeslot2 < 0 || room2 < 0) {
        return;
    }

    // 1. zamjena evenata

    schedule.placedEvents[timeslot2][room2] = -1;
    schedule.placedEvents[timeslot1][room1] = -1;

    schedule.placedEvents[timeslot2][move.room] = event1;
    schedule.placedEvents[timeslot1][move.room2] = event2;

    // 2. currentTimeslot i currentRoom

    schedule.currentTimeslot[event1] = timeslot2;
    schedule.currentRoom[event1] = move.room;

    schedule.currentTimeslot[event2] = timeslot1;
    schedule.currentRoom[event2] = move.room2;


    // 3. studentSchedule

    // ako student pohada oba eventa:
    // i dalje ima predavanje u oba;
    // tj. njemu se studentSchedule ne treba mijenjati

    std::vector<bool> attendsEvent1(schedule.S, false);
    std::vector<bool> attendsEvent2(schedule.S, false);

    for (int student : schedule.studentsOfEvent[event1]) {
        attendsEvent1[student] = true;
    }

    for (int student : schedule.studentsOfEvent[event2]) {
        attendsEvent2[student] = true;
    }


    for (int student = 0; student < schedule.S; ++student) {

        bool inEvent1 = attendsEvent1[student];
        bool inEvent2 = attendsEvent2[student];


        // student pohada samo event1
        if (inEvent1 && !inEvent2) {

            studentSchedule[student][timeslot1] = 0;
            studentSchedule[student][timeslot2] = 1;
        }

        // student pohada samo event2
        else if (!inEvent1 && inEvent2) {

            studentSchedule[student][timeslot2] = 0;
            studentSchedule[student][timeslot1] = 1;
        }
    }
}

void best_improving_neighbor(TabuSearch& schedule){

    schedule.initializePositions();

    for (int event = 0; event < schedule.numberOfEvents; ++event) {
        if (schedule.currentTimeslot[event] == -1) {
            std::cerr << "Local Search zahtijeva feasible raspored.\n";
            return;
        }
    }
    
    std::vector<std::vector<int>> StudentSchedule = 
        buildStudentSchedule(schedule.S, schedule.placedEvents, schedule.studentsOfEvent);
    while (true) {

        Move bestMove;
        int bestDelta = 0;

        findBestTransfer(schedule, bestMove, bestDelta, StudentSchedule);
        
        findBestSwap(schedule, bestMove, bestDelta, StudentSchedule);

        if (bestDelta >= 0)
            break;

        if (bestMove.method == 't')
            applyMove(schedule, bestMove, StudentSchedule);
        else if(bestMove.method == 's')
            applySwap(schedule, bestMove,StudentSchedule);
    }

}

// gleda se samo prvi poboljsavajuci susjed
// pa su komentari izostavljeni; jedina
// razlika je sto delta < bestDelta ima return;
void findFirstTransfer(
    TabuSearch& schedule, 
    Move& bestMove,
    int& bestDelta,
    std::vector<std::vector<int>>& StudentSchedule
){
    for (int event = 0; event < schedule.numberOfEvents; ++event) {

        for (int timeslot = 0; timeslot < schedule.numberOfTimeslots; ++timeslot) {

            if (timeslot == schedule.currentTimeslot[event])
               continue;

            if (schedule.eventTimeslot[event][timeslot] == 0)
               continue;


            bool conflict_exists = false;

            for(int conflict : schedule.conflictList[event])
               if(schedule.currentTimeslot[conflict] == timeslot){
                   conflict_exists = true;
                   break;
               }
            
            if (conflict_exists)
                 continue;


            for (int otherEvent = 0; otherEvent < schedule.numberOfEvents; ++otherEvent){

                if (otherEvent == event)
                   continue;

                if (schedule.currentTimeslot[otherEvent] == -1)
                    continue;

                if (schedule.precedence[otherEvent][event] == 1){
                    if (schedule.currentTimeslot[otherEvent] >= timeslot){
                        conflict_exists = true;
                        break;
                    }
                }
             
                if (schedule.precedence[event][otherEvent] == 1){
                    if (schedule.currentTimeslot[otherEvent] <= timeslot){
                        conflict_exists = true;
                        break;
                    }
                }
             }
             
            if (conflict_exists)
                    continue;
             
            int room = -1;         

            for(int i = 0; i < schedule.numberOfRooms; ++i){
                if(schedule.roomCompatible(event, i) 
                   && schedule.placedEvents[timeslot][i] == -1){
                    room = i;
                    break;
                }
            }
         
            if (room == -1)
                continue;

         
            int delta = getCostChange(event, 
                                      schedule.currentTimeslot[event],
                                      timeslot,
                                      schedule.studentsOfEvent,
                                      StudentSchedule);

         
            if (delta < bestDelta) {
                bestDelta = delta;
            
                bestMove.event = event;
                bestMove.timeslot = timeslot;
                bestMove.room = room;
                bestMove.method = 't';

                return;
            }
        }
    }
}

// isto kao i u prosloj funkciji
// razlika je sto delta < bestDelta ima return;
void findFirstSwap(
    TabuSearch& schedule,
    Move& bestMove,
    int& bestDelta,
    std::vector<std::vector<int>>& studentSchedule
) {
    for (int event1 = 0; event1 < schedule.numberOfEvents; ++event1){

        int timeslot1 = schedule.currentTimeslot[event1];

        for (int event2 = event1 + 1;
             event2 < schedule.numberOfEvents;
             ++event2)
        {
            int timeslot2 = schedule.currentTimeslot[event2];

            int room2 = schedule.currentRoom[event2];

            if (timeslot2 == -1 || room2 == -1)
                continue;

            if (timeslot1 == timeslot2)
                continue;

            if (schedule.eventTimeslot[event1][timeslot2] == 0)
                continue;


            if (schedule.eventTimeslot[event2][timeslot1] == 0)
                continue;



            int newRoom1 = -1;
            int newRoom2 = -1;        


            for(int i = 0; i < schedule.numberOfRooms; ++i){
                int occupant = schedule.placedEvents[timeslot2][i];
                if(schedule.roomCompatible(event1, i) 
                   && (occupant == -1 || occupant == event2)){
                    newRoom1 = i;
                    break;
                }
            }

            for(int i = 0; i < schedule.numberOfRooms; ++i){
                int occupant = schedule.placedEvents[timeslot1][i];
                if(schedule.roomCompatible(event2, i) 
                   && (occupant == -1 || occupant == event1)){
                    newRoom2 = i;
                    break;
                }
            }

            if (newRoom1 == -1 || newRoom2 == -1)
                continue;


            bool conflict_exists = false;


            for (int conflict : schedule.conflictList[event1])
            {

                if (conflict == event2)
                    continue;

                if (schedule.currentTimeslot[conflict]
                    == timeslot2)
                {
                    conflict_exists = true;
                    break;
                }
            }

            if (conflict_exists)
                continue;


            for (int conflict : schedule.conflictList[event2])
            {
  
                if (conflict == event1)
                    continue;

                if (schedule.currentTimeslot[conflict] == timeslot1){
                    conflict_exists = true;
                    break;
                }
            }

            if (conflict_exists)
                continue;

            auto projectedTimeslot =
                [&](int event)
                {
                    if (event == event1)
                        return timeslot2;

                    if (event == event2)
                        return timeslot1;

                    return schedule.currentTimeslot[event];
                };


            for (int otherEvent = 0; otherEvent < schedule.numberOfEvents; ++otherEvent){

                int otherTimeslot = projectedTimeslot(otherEvent);

                if (otherTimeslot == -1)
                    continue;


                if (schedule.precedence[event1][otherEvent] == 1)
                {
                    if (projectedTimeslot(event1) >= otherTimeslot)
                    {
                        conflict_exists = true;
                        break;
                    }
                }

                if (schedule.precedence[otherEvent][event1] == 1)
                {
                    if (otherTimeslot >= projectedTimeslot(event1))
                    {
                        conflict_exists = true;
                        break;
                    }
                }


                if (schedule.precedence[event2][otherEvent] == 1)
                {
                    if (projectedTimeslot(event2) >= otherTimeslot)
                    {
                        conflict_exists = true;
                        break;
                    }
                }

                if (schedule.precedence[otherEvent][event2] == 1)
                {
                    if (otherTimeslot
                        >= projectedTimeslot(event2))
                    {
                        conflict_exists = true;
                        break;
                    }
                }
            }


            if (conflict_exists)
                continue;




            int delta =
                getSwapCostChange(
                    schedule.S,
                    event1,
                    event2,
                    timeslot1,
                    timeslot2,
                    schedule.studentsOfEvent,
                    studentSchedule
                );



            if (delta < bestDelta){
                bestDelta = delta;

                bestMove.event = event1;
                bestMove.secondEvent = event2;

                bestMove.timeslot = timeslot2;
                bestMove.room = newRoom1;
                bestMove.room2 = newRoom2;
                bestMove.method = 's';

                return ;
            }
        }
    }
}



void first_improving_neighbor(TabuSearch& schedule){

    schedule.initializePositions();

    for (int event = 0; event < schedule.numberOfEvents; ++event) {
        if (schedule.currentTimeslot[event] == -1) {
            std::cerr << "Local Search zahtijeva feasible raspored.\n";
            return;
        }
    }
    
    std::vector<std::vector<int>> StudentSchedule = 
        buildStudentSchedule(schedule.S, schedule.placedEvents, schedule.studentsOfEvent);
    while (true) {

        Move bestMove;
        int bestDelta = 0;

        findFirstTransfer(schedule, bestMove, bestDelta, StudentSchedule);
        
        if (bestDelta >= 0){
            findFirstSwap(schedule, bestMove, bestDelta, StudentSchedule);
        }

        if (bestDelta >= 0)
            break;

        if (bestMove.method == 't')
            applyMove(schedule, bestMove, StudentSchedule);
        else if(bestMove.method == 's')
            applySwap(schedule, bestMove,StudentSchedule);
    }
 
}