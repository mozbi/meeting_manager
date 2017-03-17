#include "Room.h"
#include "Meeting.h"
#include "Person.h"
#include <algorithm>
#include <cassert>
#include <functional>
#include <new>

using namespace std;

// private member function that inserts a participant
// to its correct position in the participants vector.
void Room::ordered_insert_meeting(Meeting* meeting)
{
    // searches for a meeting of the specified time in the vector of meetings
    // using the lower_bound algorithm.
    auto meeting_it = lower_bound(meetings.begin(), meetings.end(), meeting, Less_than_ptr<const Meeting*>()); 
    meetings.insert(meeting_it, meeting);
}

Room::Room(ifstream& is, People_t& people_list)
{
    int num_meetings;
    is >> room_number >> num_meetings;
    file_invalid_data_check(is);
    while(num_meetings--)
    {
        Meeting* room_meeting;
        try{
            room_meeting = new Meeting(is, people_list, room_number);
        }
        catch(...)
        {
            clear_Meetings();
            throw;
        }
        ordered_insert_meeting(room_meeting);
    }
}

void Room::add_Meeting(Meeting* m)
{
    if(is_Meeting_present(m->get_time()))
    {
        throw Error(meeting_exists_at_time_message_c);
    }
    ordered_insert_meeting(m);
}

bool Room::is_Meeting_present(int time) const
{
    Meeting probe(time);
    return binary_search(meetings.begin(), meetings.end(), &probe, Less_than_ptr<const Meeting*>());
}


Meetings_t::iterator Room::get_Meeting_iter(int time)
{
    Meeting probe(time);
    // searches for a meeting of the specified time in the vector of meetings
    // using the lower_bound algorithm. 
    auto meeting_it = lower_bound(meetings.begin(), meetings.end(), &probe, Less_than_ptr<const Meeting*>()); 
    // if the iterator returned points to end or a meeting whose time is
    // different, we throw an exception. 
    if (meeting_it == meetings.end() || (*meeting_it)->get_time() != time)
    {
        throw Error(no_meeting_at_time_message_c);
    } 
    return meeting_it;
}

Meeting* Room::get_Meeting_private(int time) 
{
    auto meeting_it = get_Meeting_iter(time);
    assert(meeting_it != meetings.end());
    return *meeting_it;
}
 
const Meeting* Room::get_Meeting(int time) 
{
    const Meeting* meeting = get_Meeting_private(time);
    return meeting;
}
 

// Remove the specified Meeting, throw exception if a Meeting at that time was not found.
Meeting* Room::remove_Meeting(int time)
{
    auto meeting_it = get_Meeting_iter(time);
    assert(meeting_it != meetings.end());
    Meeting* removed_meeting = *meeting_it;
    // erase the meeting to be removed from the vector of meetings.
    meetings.erase(meeting_it);
    return removed_meeting; 
}

void Room::add_Meeting_participant(int time, Person* person)
{
    Meeting* meeting = get_Meeting_private(time);
    assert(meeting);
    if(meeting->is_participant_present(person))
    {
        throw Error("This person is already a participant!");
    }

    person->add_commitment(room_number, meeting);
    meeting->add_participant(person);
}
                                                       
void Room::remove_Meeting_participant(int time, Person* person)
{
    Meeting* meeting = get_Meeting_private(time);
    assert(meeting);
    meeting->remove_participant(person);
    person->remove_commitment(room_number, time);
}

void Room::clear_Meetings()
{
    // deletes each of the meetings in the vector of meetings.
    for_each(meetings.begin(), meetings.end(), [](Meeting* m){delete m;});
    meetings.clear();
}


// Return true if the person is present in any of the meetings
bool Room::is_participant_present(const Person* person_ptr) const
{
    // searches each meeting in the vector of meetings for a participant and returns true
    // if any of them contains a participant.
    return any_of(meetings.begin(), meetings.end(), 
            bind(&Meeting::is_participant_present, placeholders::_1, person_ptr));
}

// Write a Rooms's data to a stream in save format, with endl as specified.
void Room::save(ostream& os) const
{
    os << room_number << " " << get_number_Meetings() << endl;
    // saves each meeting in the vector of meetings.
    for_each(meetings.begin(), meetings.end(), 
            bind(&Meeting::save, placeholders::_1, ref(os)));
}

ostream& operator<< (ostream& os, const Room& room)
{
    os << "--- Room " << room.room_number <<  " ---" << endl;
    if (room.meetings.empty())
    {
        os << "No meetings are scheduled" << endl;
    }
    else
    {
        // print each meeting in the vector of meetings.
        for_each(room.meetings.begin(), room.meetings.end(),
                [&os](const Meeting* meeting){os << *meeting;});
    }
    return os;
}
