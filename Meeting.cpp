#include "Meeting.h"
#include "Person.h"
#include <algorithm>
#include <iterator>
#include <functional>

using namespace std;

Meeting::Meeting(ifstream& is, const People_t& people, int room_number)
{
    int num_participants;
    is >> time >> topic >> num_participants;
    file_invalid_data_check(is);
    
    while(num_participants--)
    {
        string lastname;
        is >> lastname;
        file_invalid_data_check(is);
        // Insert person in people's list into participants list if person exists.
        Person probe(lastname);
        auto people_it = people.find(&probe);
        if (people_it == people.end())
        {
            throw Error(invalid_file_data_message_c);
        }
        add_participant(*people_it);
        // add the commitment for the participant in this room.
        (*people_it)->add_commitment(room_number, this);
    }
}

void Meeting::add_participant(const Person* p)
{
    // searches and gets and iterator to  the first occurence of a 
    // person in the list whose lastname is larger than p 
    auto insert_it = find_if(participants.begin(), participants.end(), 
            bind(Less_than_ptr<const Person*>(), p, placeholders::_1)); 
    participants.insert(insert_it, p);
}

bool Meeting::is_participant_present(const Person* p) const
{
    // if find returns end, participant is not present. 
    return find(participants.begin(), participants.end(), p) != participants.end();
}

void Meeting::remove_participant(const Person* p)
{
    // linear search for list
    if(find(participants.begin(), participants.end(), p) == participants.end())
    {
        throw Error("This person is not a participant in the meeting!");
    }
    participants.remove(p);
}
        
void Meeting::save(ostream& os) const
{
    os << time << " " << topic << " " << participants.size() << endl;
    for_each(participants.begin(), participants.end(), 
            [&os](const Person* person){os << person->get_lastname() << endl;});
}

bool Meeting::has_participant_commitment_conflict(int old_meeting_time, int new_meeting_time) const
{
    // Changing the room but not the time should not cause
    // a commitment conflict.
    if (old_meeting_time == new_meeting_time)
    {
        return false;
    }

    // returns true if any of the participants has a commitment conflict.
    return any_of(participants.begin(), participants.end(), 
            bind(&Person::has_commitment_conflict, placeholders::_1, new_meeting_time));
}

int Meeting::format_time(const int time) const
{
    /* Time should be from 9am to 5pm. */
    return (time<=5) ? time+12 : time;
}
// This operator defines the order relation between meetings, based just on the time
bool Meeting::operator< (const Meeting& other) const
{
    return (format_time(time) < format_time(other.time));
}

ostream& operator<< (ostream& os, const Meeting& meeting)
{
    os << "Meeting time: " << meeting.time << ", Topic: " << meeting.topic << "\nParticipants:";
    if (meeting.participants.empty())
    {
        os << " None" << endl;
    }
    else
    {
        os << endl;
        // initialize ostream iterator with newline
        ostream_iterator<const Person*> os_it(os, "\n");
        // copy each participant into the ostream iterator.
        copy(meeting.participants.begin(),
                meeting.participants.end(), os_it);
    }
    return os;
}
