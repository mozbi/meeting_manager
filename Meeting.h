#ifndef MEETING_H
#define MEETING_H

#include "Utility.h"
#include <fstream>
#include <string>
#include <list>

class Person;

class Meeting {
public:
    Meeting(int time_, const std::string& topic_) :
        participants(Participants_t()),
        time(time_),
        topic(topic_) {}
	// construct a Meeting with only a time
	Meeting(int time_) :
        participants(Participants_t()),
        time(time_),
        topic(std::string()) {}

    // Construct a Meeting from an input file stream in save format
    // Throw Error exception if invalid data discovered in file.
    // No check made for whether the Meeting already exists or not.
    // Person list is needed to resolve references to meeting participants
    // Input for a member variable value is read directly into the member variable.
    // Sets commitments for people in the meeting. room_number parameter
    // is used in commitments to know which room a meeting is in.
    Meeting(std::ifstream& is, const People_t& people, int room_number);

    // accessors
    int get_time() const
        { return time; }
    std::string get_topic() const
        { return topic; }
	void set_time(int time_)
		{ time = time_; }

    // Meeting objects manage their own participant list. Participants
    // are identified by a pointer to that individual's Person object.

    // Add to the list, throw exception if participant was already there.
    void add_participant(const Person* p);
    // Return true if the person is a participant, false if not.
    bool is_participant_present(const Person* p) const;
    // Remove from the list, throw exception if participant was not found.
    void remove_participant(const Person* p);
			
    // Write a Meeting's data to a stream in save format with final endl.
    void save(std::ostream& os) const;

    // Checks all the participants in the meeting for a commitment conflict
    // given the old and new meeting times. This is used when meetings are being
    // rescheduled.
    bool has_participant_commitment_conflict(int old_meeting_time, int new_meeting_time) const;


    // This operator defines the order relation between meetings, based just on the time
    bool operator< (const Meeting& other) const;
	
    friend std::ostream& operator<< (std::ostream& os, const Meeting& meeting);
		
private:
    // converts time to 24hr format for comparison.
    int format_time(const int time) const;
    // a list of const person pointers to store participants for this meeting.
    using Participants_t = std::list<const Person*>;
    Participants_t participants;

    int time;
    std::string topic;
};

// Print the Meeting data as follows:
// The meeting time and topic on one line, followed either by:
// the no-participants message with an endl
// or the partipcants, one per line, with an endl after each one
std::ostream& operator<< (std::ostream& os, const Meeting& meeting);

#endif
