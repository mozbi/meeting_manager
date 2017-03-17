#include "Person.h"
#include "Meeting.h"
#include <algorithm>
#include <iostream>

using namespace std;

Person::Person(ifstream& is)
{
    is >> firstname >> lastname >> phoneno;
    file_invalid_data_check(is);
}

void Person::save(ostream& os) const
{
    os << firstname << " " << lastname << " " << phoneno << endl;
}

void Person::add_commitment(int room_number, const Meeting* meeting)
{
    // check for commitment conflicts before adding a new commitment.
    if (has_commitment_conflict(meeting->get_time()))
    {
        throw Error("Person is already committed at that time!");
    }

    auto commitment_it = commitments.find(room_number);
    // if a room does not exist in the commitment map, create
    // a new entry for it.
    if (commitment_it  == commitments.end())
    {
        commitments.emplace(
            make_pair(room_number, set<const Meeting*, Less_than_ptr<const Meeting*>>{meeting}));
        return;
    }

    // insert a meeting to the commited room if its key already
    // exists in the map.
    commitment_it->second.insert(meeting);

}

bool Person::remove_commitment(int room_number, int meeting_time)
{
    auto room_commitment_it = commitments.find(room_number);
    if (room_commitment_it == commitments.end())
    {
        // person is not commited to room so return false.
        return false;
    }

    Meeting probe(meeting_time);
    auto meeting_it = room_commitment_it->second.find(&probe);
    if (meeting_it == room_commitment_it->second.end())
    {
        // person not committed to specified meeting_it time
        // in the room.
        return false;
    }

    room_commitment_it->second.erase(meeting_it);
    // remove room from map if there are no committed
    // meeting_its in it.
    if (room_commitment_it->second.empty())
    {
        commitments.erase(room_commitment_it);
    }
    return true;
}

void Person::remove_room_commitments(int room_number)
{
    auto room_committed = commitments.find(room_number);
    // remove a committed room from the map of commitments.
    // What this means is we are no longer committed to any
    // of the meetings in the room that is removed.
    if (room_committed != commitments.end())
    {
        commitments.erase(room_committed);
    }
}

void Person::print_commitment() const
{
    if (commitments.empty())
    {
        cout << "No commitments" << endl;
        return;
    }
    // go through each key in the commitments map (room number)
    for_each(commitments.begin(), commitments.end(),
            [](Commitments_t::value_type room_meet_pair)
            {
                // go through each meeting in the set of meetings for the room
                // and print the commitments
                for_each(room_meet_pair.second.begin(), room_meet_pair.second.end(),
                    [&room_meet_pair](const Meeting* meeting)
                    {
                        cout << "Room:" << room_meet_pair.first
                        << " Time: " << meeting->get_time()
                        << " Topic: " << meeting->get_topic() << endl;
                    });
            });

}

bool Person::has_commitment_conflict(int time) const
{
    // go through each key in the commitments map (room number)
    return any_of(commitments.begin(), commitments.end(),
            [&time](Commitments_t::value_type room_meet_pair)
            {
                // check if any of the meetings in the room conflict with the provided time.
                return any_of(room_meet_pair.second.begin(), room_meet_pair.second.end(),
                    [&time](const Meeting* meeting)
                    {return meeting->get_time() == time;});
            });
}

ostream& operator<< (ostream& os, const Person& person)
{
    os << person.firstname << " " << person.lastname << " " << person.phoneno;
    return os;
}

ostream& operator<< (ostream& os, const Person* person)
{
    os << person->firstname << " " << person->lastname << " " << person->phoneno;
    return os;
}
