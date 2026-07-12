#include <iostream>
#include "parser.h"

using namespace std;

int main()
{
    try
    {
        int i;
        cout << "Izaberi dataset izmedju 1 i 24: ";
        cin >> i;
        cout << endl;

        TimData data = loadTIM("datasets/dataset" + to_string(i) + ".tim");

        cout << "Events: " << data.E << endl;
        cout << "Rooms: " << data.R << endl;
        cout << "Features: " << data.F << endl;
        cout << "Students: " << data.S << endl;
        
        // dodatni debug
        // cout << "Timeslots: " << (data.eventTimeslot.empty() ? 0 : data.eventTimeslot[0].size()) << endl;
        // cout << "roomSizes[0]=" << (data.roomSizes.empty() ? -1 : data.roomSizes[0]) << endl;
        // cout << "studentEvent[0][0]=" << data.studentEvent[0][0] << endl;
        // cout << "eventTimeslot[0][0]=" << (data.eventTimeslot.empty() ? -1 : data.eventTimeslot[0][0]) << endl;

        // if (!data.precedence.empty())
        // {
        //     cout << "precedence[0][1]=" << data.precedence[0][1] << endl;
        // }

    }
    catch (const exception& error)
    {
        cerr << "Error: " << error.what() << endl;
        return 1;
    }

    return 0;
}