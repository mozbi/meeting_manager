#ifndef ROOM_H
#define ROOM_H

#include "Utility.h"
#include <ostream>
#include <vector>

class Meeting;

using Meetings_t = std::vector<Meeting*>;
/* A Room object contains a room number and a list containing Meeting objects stored with
meeting times as the key.  When created, a Room has no Meetings. When destroyed, the Meeting
objects in a Room are automatically destroyed.

Rooms manage the Meetings contained in them; functions are present for finding, adding,
or removing a Meeting specified by time.  The get_Meeting function returns a reference to the
specified Meeting, so that client code can modify the meeting - e.g. by adding a participant.
Note that modifying the time for a meeting in the container will disorder the meeting container, 
and so should not be attempted.

In addition, a Room can be asked to search for a particular Person being a participant 
in any of the Meetings in the Room. This makes it unnecessary for client code 
to be able to access the Meeting container in order to search for a specific participant.

We let the compiler supply the destructor and copy/move constructors and assignment operators.
*/ 

class Room {
public:
    // Construct a room with the specified room number and no meetings
    Room(int room_number_) : meetings(Meetings_t()), room_number(room_number_) {}
    // Construct a Room from an input file stream in save format, using the people list,
    // restoring all the Meeting information. 
    // Person list is needed to resolve references to meeting participants.
    // No check made for whether the Room already exists or not.
    // Throw Error exception if invalid data discovered in file.
    // Input for a member variable value is read directly into the member variable.
     Room(std::ifstream& is, People_t& people_list);


    // Accessors
    int get_room_number() const
        { return room_number; }
                    
    // Room objects manage their own Meeting container. Meetings are objects in
    // the container. The container of Meetings is not available to clients.

    // Add the Meeting, throw exception if there is already a Meeting at that time.
    // A copy of the supplied Meeting is stored in the Meeting container.
    void add_Meeting(Meeting* m);
    // Return true if there is at least one meeting, false if none
    bool has_Meetings() const
        {return !meetings.empty();}		
    // Return the number of meetings in this room
    int get_number_Meetings() const
        {return meetings.size();}		
    // Return true if there is a Meeting at the time, false if not.
    bool is_Meeting_present(int time) const;
    // Return a const pointer to a Meeting if it is present, throw exception if not.
    const Meeting* get_Meeting(int time);
    // Remove the specified Meeting, throw exception if a Meeting at that time was not found. 
    // Returns a pointer to the meeting that was removed.
    Meeting* remove_Meeting(int time);
    
    // Add a participant to the meeting in this room specified by
    // the time. Also adds a meeting commitment for the person being added as a participant.
    // Throw exception if participant is already present in the meeting.
    void add_Meeting_participant(int time, Person* person);
    // Remove a participant from the meeting in this room specified by the time.
    // Also removes a meeting commitment for the person.
    void remove_Meeting_participant(int time, Person* person);

    // Clears and deallocates the meetings in this room.
    void clear_Meetings();

    // Return true if the person is present in any of the meetings
    bool is_participant_present(const Person* person_ptr) const;

    // Write a Rooms's data to a stream in save format, with endl as specified.
    void save(std::ostream& os) const;

    // This operator defines the order relation between Rooms, based just on the number
    bool operator< (const Room& rhs) const
        {return room_number < rhs.room_number;}		

    /* *** provide a friend declaration for the output operator */
    friend std::ostream& operator<< (std::ostream& os, const Room& room);

private:
    
    // private member function that inserts a participant to
    // its correct position in the participants vector.
    void ordered_insert_meeting(Meeting* meeting);

    // returns a meeting to the iterator of the specified time.
    Meetings_t::iterator get_Meeting_iter(int time);

    // Returns a non-const pointer to a meeting at a specified time.
    // This is to allow the room class to manage its meetings directly.
    Meeting* get_Meeting_private(int time);

    // vector of meeting pointers to store meetings in the room.
    Meetings_t meetings;

    int room_number;

};

// Print the Room data as follows:
// The room heading with room number followed by an endl, followed by either:
// The no-meetings message with an endl, or
// The information for each meeting, which should automatically have a final endl
std::ostream& operator<< (std::ostream& os, const Room& room);

#endif
