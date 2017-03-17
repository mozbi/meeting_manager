#include "Utility.h"
#include "Person.h"
#include "Meeting.h"
#include "Room.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <new>
#include <functional>
#include <map>
#include <vector>

using namespace std;
// alias for vector of rooms
using Room_t = vector<Room>;

/*
 * struct containing variables needed by command functions.
 * used as a short-hand to move all variables at once.
 */
struct MeetingData
{
    Room_t& rooms;
    People_t& people;

    MeetingData(Room_t& rooms_, People_t& people_)
        : rooms(rooms_), people(people_) {}
};   

// class that overloads the function operator
// to calculate the sum of the meetings of all the rooms
// by incrementing sum each time it is called with a room
class Calc_Sum_Meetings {

    public:
        Calc_Sum_Meetings() : sum(0) {}
        void operator() (Room& room)
        {
            sum += room.get_number_Meetings();
        }

        int get_sum() const
        {                   
            return sum;
        }

    private:
        int sum;
};   

// string literals  
const char* const no_person_message_c = "No person with that name!";
const char* const invalid_command_message_c = "Unrecognized command!";
const char* const type_not_integer_message_c = "Could not read an integer value!";
const char* const bad_room_range_message_c = "Room number is not in range!";
const char* const bad_time_range_message_c = "Time is not in range!";
const char* const no_room_number_message_c = "No room with that number!";     
const char* const person_is_participant_message_c = "This person is a participant in a meeting!";
const char* const file_cannot_open_message_c = "Could not open file!"; 
const char* const enter_cmd_message_c = "\nEnter command: "; 
const char* const all_persons_deleted_message_c = "All persons deleted";
const char* const all_meetings_deleted_message_c = "All meetings deleted";


// Prototypes for functions that handle print commands and their helpers. 
static Person* find_and_get_person(People_t& people);
static void cmd_print_individual(MeetingData& meeting_data);
static void cmd_print_person_commitments(MeetingData& meeting_data);
static int read_and_check_cmd_int();
static int get_and_check_room_number();
static vector<Room>::iterator find_room_it(Room_t& rooms, int room_number);
static Room& find_room(Room_t& rooms, int room_number);
static void cmd_print_room(MeetingData& meeting_data);
static int get_and_check_meeting_time();
static void cmd_print_meeting(MeetingData& meeting_data);
static void cmd_print_all_meetings(MeetingData& meeting_data);
static void cmd_print_all_people(MeetingData& meeting_data);
static void cmd_print_allocated(MeetingData& meeting_data);

// Prototypes for functions that handle add commands. 
static void cmd_add_individual(MeetingData& meeting_data);
static void cmd_add_room(MeetingData& meeting_data);
static void cmd_add_meeting(MeetingData& meeting_data);
static void cmd_add_participant(MeetingData& meeting_data);

// Prototype for reschedule meeting command. 
static void cmd_reschedule_meeting(MeetingData& meeting_data);

// Prototypes for functions that handle delete commands and their helpers. 
static void cmd_delete_individual(MeetingData& meeting_data);
static void cmd_delete_room(MeetingData& meeting_data);
static void cmd_delete_meeting(MeetingData& meeting_data);
static void cmd_delete_participant(MeetingData& meeting_data);
static void cmd_delete_schedule(MeetingData& meeting_data);
static void clear_people_list(People_t& people);
static void cmd_delete_all_individuals(MeetingData& meeting_data);
static void clear_room_list(Room_t& rooms);
static void cmd_delete_all(MeetingData& meeting_data);

/* 
 * Prototypes for functions that handle save and load commands and their
 * helpers. 
 */
static void cmd_save_data(MeetingData& meeting_data);
static void roll_back(Room_t& rooms, People_t& people, Room_t& rooms_backup, People_t& people_backup, ifstream& is);
static void cmd_load_data(MeetingData& meeting_data);
static void cmd_quit(MeetingData& meeting_data);


int main(int argc, char* argv[])
{
    char input_cmd_first, input_cmd_second;
    Room_t rooms;
    People_t people;

    MeetingData meeting_data(rooms, people);

    // map of commands to their function pointers
    map<string, void(*)(MeetingData&)> cmd_mapper 
    {
        {"pi", cmd_print_individual},
        {"pc", cmd_print_person_commitments},
        {"pr", cmd_print_room},
        {"pm", cmd_print_meeting},
        {"ps", cmd_print_all_meetings},
        {"pg", cmd_print_all_people},
        {"pa", cmd_print_allocated},
        {"ai", cmd_add_individual},
        {"ar", cmd_add_room},
        {"am", cmd_add_meeting},
        {"ap", cmd_add_participant},
        {"rm", cmd_reschedule_meeting},
        {"di", cmd_delete_individual}, 
        {"dr", cmd_delete_room},
        {"dm", cmd_delete_meeting},
        {"dp", cmd_delete_participant},
        {"ds", cmd_delete_schedule},
        {"dg", cmd_delete_all_individuals},
        {"da", cmd_delete_all},
        {"sd", cmd_save_data},
        {"ld", cmd_load_data}
    };

    while(true)
    {
        cout << enter_cmd_message_c;
        cin >> input_cmd_first >> input_cmd_second;

        string cmd = "";
        cmd += input_cmd_first;
        cmd += input_cmd_second;

        // quit command
        if (cmd == "qq")
        {
            cmd_quit(meeting_data);
            return 0;
        }

        try{
            auto cmd_func = cmd_mapper.find(cmd);
            if (cmd_func == cmd_mapper.end())
            {
                throw Error(invalid_command_message_c);
            }
            cmd_func->second(meeting_data);
        }
        // catch internal errors thrown
        catch(Error& e)
        {
            while(cin.get() != '\n');    /* skip rest of line */
            cout << e.msg << endl;
        }
        // catch exception thrown by new
        catch(bad_alloc& ba)
        {
            cerr << "bad_alloc exception caught!" << endl;
            cmd_quit(meeting_data);
            return 0;
        }
        //catch all other errors
        catch(...)
        {
            cerr << "Unknown exception caught!" << endl;
            cmd_quit(meeting_data);
            return 0;
        }
    }
    return 0;
}

/* 
 * Reads a string from input corresponding to a person's lastname
 * and checks if a person with that lastname exists in a the
 * people list. Returns a pointer to the person if the person
 * is in the people's list and throws an error if not.
 */
static Person* find_and_get_person(People_t& people)
{
    string lastname;
    cin >> lastname;
    Person person(lastname);
    auto person_it = people.find(&person);
     
    // Throws an error message if the individual to be added is not found
    // in the people's list.
    if (person_it == people.end())
    {
        throw Error(no_person_message_c);
    }
    return *person_it;
}
/*
 * Called when a user of the program types in the 'pi' command.
 * Prints the specified indiviual information of the person.
 * Errors: No person with the passed in last name.
 */
static void cmd_print_individual(MeetingData& meeting_data)
{
    const Person* person = find_and_get_person(meeting_data.people);
    assert(person);
    cout << *person << endl;
}

/*
 * Called when the user of the program types the 'pc' command.
 * Prints the commitments of a person given the person's lastname.
 * Errors: No person with the passed in last name.
 */
static void cmd_print_person_commitments(MeetingData& meeting_data)
{
    const Person* person = find_and_get_person(meeting_data.people);
    assert(person);
    person->print_commitment();
}
/*
 * Reads an integer from standard input. Whitespaces are ignored.
 * If the datum read is not an integer, an error message is printed
 * and unwanted characters in the input following the character 
 * are skipped. Throws an error if the input type is not an integer.
 */
static int read_and_check_cmd_int()
{
    /* Reads one integer and checks that scanf returns 1. */
    int cmd;
    cin >> cmd;
    if(!cin)
    {
        cin.clear();
        throw Error(type_not_integer_message_c);
    }
    return cmd;
}

/*
 * Performs error checks on integer pointed to by the argument.
 * The input data is first checked for whether it is an integer,
 * and if it is, whether it is in the valid range for a room number.
 * Returns the room number if the above checks pass; otherwise, throw
 * an error.
 */
static int get_and_check_room_number()
{
    /* Checks if data read is an integer. */
    int room_number = read_and_check_cmd_int();

    /* Checks if room number is in valid range (>0) */
    if(room_number <= 0)
    {
        throw Error(bad_room_range_message_c);
    }
    return room_number;
}

static vector<Room>::iterator find_room_it(Room_t& rooms, int room_number)
{
    /* check if room exists. */
    Room room(room_number);
    auto room_it = lower_bound(rooms.begin(), rooms.end(), room);
    if(room_it == rooms.end() || room_it->get_room_number() != room_number)
    {
        throw Error(no_room_number_message_c);
    } 
    assert(room_it != rooms.end());
    return room_it;
}
 
static Room& find_room(Room_t& rooms, int room_number)
{
    auto room_it = find_room_it(rooms, room_number);
    assert(room_it != rooms.end());
    return *room_it;
}
/*
 * Called when a user of the program types in the 'pr' command.
 * Prints the meeting in a room with the specified number.
 * Errors: Room number out of range, no room of that number.
 */ 
static void cmd_print_room(MeetingData& meeting_data)
{
    int room_number = get_and_check_room_number();
    // get room based on valid room number
    Room& room = find_room(meeting_data.rooms, room_number);
    cout << room;
}

/*
 * Performs error checks on integer pointed to by the argument.
 * The input data is first checked for whether it is an integer,
 * and if it is, whether it is in the valid range for a time.
 * Returns 1 if there are no errors and returns 0 if an error occured.
 */ 
static int get_and_check_meeting_time()
{
    int time = read_and_check_cmd_int();

    /* Time is in valid range if it is from 9 to 5 in 12hr format. */
    if(!((time >= 9 && time <= 12) || (time >= 1 && time <= 5)))
    {
        throw Error(bad_time_range_message_c);
    }
    return time;
}

/*
 * Called when a user of the program types in the 'pm' command.
 * Prints the time, topic, and participants for a specified meeting.
 * Errors: room number out of range, no room of that number, time out
 * of range, no meeting at that time.
 */ 
static void cmd_print_meeting(MeetingData& meeting_data)
{
    int room_number = get_and_check_room_number();
    Room& room = find_room(meeting_data.rooms, room_number);
    int time = get_and_check_meeting_time();
    
    // get_Meeting checks for presence of meeting.
    const Meeting* meeting = room.get_Meeting(time);
    assert(meeting);
    cout << *meeting;
}

/*
 * Called when a user of the program types in the 'ps' command.
 * Prints the meeting information for all meetings in a room.
 * Errors: None.
 */ 
static void cmd_print_all_meetings(MeetingData& meeting_data)
{
    if (meeting_data.rooms.empty())
    {
        cout << "List of rooms is empty" << endl;;
        return;
    }
    else
    {
        cout << "Information for "<< meeting_data.rooms.size() << " rooms:" << endl;
        /* Prints room information for each room. */
        /**** the one range for ***/
        for(const auto& room : meeting_data.rooms)
        {
            cout << room;
        }
    }
}

/*
 * Called when a user of the program types in the 'pg' command.
 * Prints the individual information for all people in the person list.
 * Errors: None.
 */ 
static void cmd_print_all_people(MeetingData& meeting_data)
{
    if (meeting_data.people.empty())
    {
        cout << "List of people is empty" << endl;;    
    }
    else
    {
        cout << "Information for "<< meeting_data.people.size() << " people:" << endl;
        // prints information for each person in the people list.
        for_each(meeting_data.people.begin(), meeting_data.people.end(), 
                [](const Person* person){ cout << *person << endl;});
    }
}

/*
 * Called when a user of the program types in the 'pa' command.
 * Prints all memory allocations.
 * Errors: None.
 */ 
static void cmd_print_allocated(MeetingData& meeting_data)
{
    cout << "Memory allocations:" << endl;
    cout << "Persons: " << meeting_data.people.size() << endl;
    // creates a functor that we use to get the sum of the meeting from.
    // each room's number of meetings is added to the function object
    Calc_Sum_Meetings cs = for_each(meeting_data.rooms.begin(), meeting_data.rooms.end(), Calc_Sum_Meetings());
    cout << "Meetings: " << cs.get_sum() << endl;
    cout << "Rooms: " << meeting_data.rooms.size() << endl;
}

/*
 * Called when a user types in the 'ai' command.
 * Adds an individual person to the people list.
 * Errors: Person with last name already in people list.
 */
static void cmd_add_individual(MeetingData& meeting_data)
{
    string firstname, lastname, phoneno;
    /* read all 3 last names before testing validity */
    cin >> firstname >> lastname >> phoneno;

    Person* person;
    try
    {
        // if throw, main catches bad alloc.
        person = new Person(firstname, lastname, phoneno);
    }
    catch (bad_alloc& ba)
    {
        throw;
    }

    if (meeting_data.people.find(person) != meeting_data.people.end())
    {
        delete person;
        throw Error("There is already a person with this last name!");
    }

    meeting_data.people.insert(person);
    cout << "Person " << lastname << " added" << endl;
}

/*
 * Called when a user types the 'ar' command.
 * Adds a room with the specified number.
 * Errors: Room number out of range, room of that number already exists.
 */
static void cmd_add_room(MeetingData& meeting_data)
{
    int room_number = get_and_check_room_number();
    
    Room room(room_number);
    auto room_it = lower_bound(meeting_data.rooms.begin(), meeting_data.rooms.end(), room);
    if (room_it != meeting_data.rooms.end() && room_it->get_room_number() == room_number)
    {
        throw Error("There is already a room with this number!");
    }
    meeting_data.rooms.insert(room_it, room);
    cout << "Room " << room_number << " added" << endl;
}

/*
 * Called when a user types the 'am' command.
 * Adds a meeting in a specified room, at a specified time, and
 * on a specified topic.
 * Errors: room number out of range, no room of that number, time
 * out of range, no meeting at that time, no person in people list of
 * that name, there is already a participant of that name.
 */
static void cmd_add_meeting(MeetingData& meeting_data)
{
    int room_number = get_and_check_room_number();
    Room& room = find_room(meeting_data.rooms, room_number);

    int time = get_and_check_meeting_time();

    string topic;
    cin >> topic;
    
    Meeting* meeting = new Meeting(time, topic);

    /* add handles duplicate meeting error. */
    try{
        room.add_Meeting(meeting);
    }
    catch(...)
    {
        delete meeting;
        throw;
    }
    cout << "Meeting added at " << time << endl;
}

/*
 * Called when a user types the 'ap' command.
 * Adds a specified person as a participant in a specified meeting.
 * Errors: Room number out of range, no room of that number, time out of range,
 * no meeting at time, no person in people list of that name,
 * participant already exists for that time.
 */
static void cmd_add_participant(MeetingData& meeting_data)
{
    int room_number = get_and_check_room_number();

    Room& room = find_room(meeting_data.rooms, room_number);

    int time = get_and_check_meeting_time();
    /* check meeting exists */
    if (!room.is_Meeting_present(time))
    {
        throw Error(no_meeting_at_time_message_c);
    }
    Person* person = find_and_get_person(meeting_data.people);
    assert(person);

    room.add_Meeting_participant(time, person);
    
    cout << "Participant " << person->get_lastname() << " added" << endl;
}

/*
 * Called when user types the 'rm' command.
 * Reschedules a meeting by changing its room and/or time
 * without changing or reentering topic or participants.
 */
static void cmd_reschedule_meeting(MeetingData& meeting_data)
{
    int old_room_number = get_and_check_room_number();
    Room& old_room = find_room(meeting_data.rooms, old_room_number);

    int old_meeting_time = get_and_check_meeting_time();
    // also checks whether meeting is present.
    const Meeting* old_room_meeting = old_room.get_Meeting(old_meeting_time);
    assert(old_room_meeting);

    int new_room_number = get_and_check_room_number();
    Room& new_room = find_room(meeting_data.rooms, new_room_number);

    int new_meeting_time = get_and_check_meeting_time();
    // rescheduling to the same room and time, print message and return.
    if (old_meeting_time == new_meeting_time && old_room_number == new_room_number)
    {
        cout << "No change made to schedule" << endl;
        return;
    }

    //check that new time is available for meeting in new room.
    if (new_room.is_Meeting_present(new_meeting_time))
    {
        throw Error(meeting_exists_at_time_message_c);
    }

    // check for participant conflicts
    if (old_room_meeting->has_participant_commitment_conflict(old_meeting_time, new_meeting_time))
    {
        throw Error("A participant is already committed at the new time!");
    }

    // add participants whose commitments are to be rescheduled into a vector
    vector<Person*> participants_to_reschedule;
    for_each(meeting_data.people.begin(), meeting_data.people.end(),
            [old_room_number, old_meeting_time, &participants_to_reschedule](Person* p)
            {
                // remove commitments that are to be rescheduled
                bool removed = p->remove_commitment(old_room_number, old_meeting_time);
                if(removed)
                {
                    participants_to_reschedule.push_back(p);
                }
            });

    Meeting* meeting_to_reschedule = old_room.remove_Meeting(old_meeting_time);
    assert(meeting_to_reschedule);
    meeting_to_reschedule->set_time(new_meeting_time);
    new_room.add_Meeting(meeting_to_reschedule);

    // for the participants whose commitments are rescheduled, add the new
    // meeting to their commitments.
    for_each(participants_to_reschedule.begin(), participants_to_reschedule.end(),
            bind(&Person::add_commitment, placeholders::_1, new_room_number, meeting_to_reschedule));

    cout << "Meeting rescheduled to room " << new_room_number << " at " << new_meeting_time << endl;
}

/*
 * Called when the user types a 'di' command.
 * Delets a person from the people list, but only if the person is not
 * a participant in the meeting.
 * Errors: No person of that name or person is a participant in a meeting.
 *
 */
static void cmd_delete_individual(MeetingData& meeting_data)
{
    Person* person = find_and_get_person(meeting_data.people);
    assert(person);
    
    if(any_of(meeting_data.rooms.begin(), meeting_data.rooms.end(), 
                bind(&Room::is_participant_present, placeholders::_1, person)))
    {
        throw Error(person_is_participant_message_c);
    }

    cout << "Person " << person->get_lastname() << " deleted" << endl;
    meeting_data.people.erase(person);
    delete person;
}

/*
 * Called when the user types a 'dr' command.
 * Deletes the room with the specified number, including
 * all meetings scheduled in the room.
 * Errors: room number out of range, no room of that number.
 */
static void cmd_delete_room(MeetingData& meeting_data)
{
    int room_number = get_and_check_room_number();

    /* check if room exists. */
    auto room_it = find_room_it(meeting_data.rooms, room_number);
    assert(room_it != meeting_data.rooms.end());
    // need to clear meetings in a room to free pointers
    room_it->clear_Meetings();
    meeting_data.rooms.erase(room_it);
    cout << "Room " << room_number << " deleted" << endl;
    for_each(meeting_data.people.begin(), meeting_data.people.end(), 
            bind(&Person::remove_room_commitments, placeholders::_1, room_number));
}

/*
 * Called when the user types a 'dm' command.
 * Deletes a meeting.
 * Errors: room number out of range, no room of that number,
 * time out of range, no meeting at that time.
 */
static void cmd_delete_meeting(MeetingData& meeting_data)
{
    int room_number = get_and_check_room_number();
    Room& room = find_room(meeting_data.rooms, room_number);

    int time = get_and_check_meeting_time();

    Meeting* removed_meeting = room.remove_Meeting(time);
    assert(removed_meeting);

    // removes commitments to this meeting for all participants who are
    // committed.
    for_each(meeting_data.people.begin(), meeting_data.people.end(), 
            bind(&Person::remove_commitment, placeholders::_1, room_number, time));

    delete removed_meeting;
    cout << "Meeting at " << time << " deleted" << endl;
}

/*
 * Called when the user types a 'dp' command.
 * Delete a specified person from the participant list for a specified meeting.
 * Errors: room num out of range, no room of the num, time out of range,
 * no meeting at time, no person of that name in people list,
 * no person of name in participant list.
 */
static void cmd_delete_participant(MeetingData& meeting_data)
{
    int room_number = get_and_check_room_number();

    /*check room exists */
    Room& room = find_room(meeting_data.rooms, room_number);
    int time = get_and_check_meeting_time();

    /* check meeting exists */
    if (!room.is_Meeting_present(time))
    {
        throw Error(no_meeting_at_time_message_c);
    }
    Person* person = find_and_get_person(meeting_data.people);
    assert(person);
    room.remove_Meeting_participant(time, person);
    cout << "Participant " << person->get_lastname() << " deleted" << endl;
}

/*
 * Called when the user enters a 'ds' command.
 * Delete all meetings from all room.
 * Errors: None.
 */
static void cmd_delete_schedule(MeetingData& meeting_data)
{
    /* clear all the meeting_data.rooms in the container */
    for_each(meeting_data.rooms.begin(), meeting_data.rooms.end(), mem_fn(&Room::clear_Meetings));
    for_each(meeting_data.people.begin(), meeting_data.people.end(), mem_fn(&Person::clear_Commitments));
    cout << all_meetings_deleted_message_c << endl;
}

/*
 * Function that deletes people pointers and clears the people list.
 */
static void clear_people_list(People_t& people)
{
    for_each(people.begin(), people.end(), 
            [](const Person* person){ delete person; });
    people.clear();
}

/*
 * Called when the user enters a 'dg' command.
 * Deletes all of the individual information, but only if
 * there are no meetings scheduled.
 * Errors: There are scheduled meetings.
 */
static void cmd_delete_all_individuals(MeetingData& meeting_data)
{
    /* for each room, look at each meeting's size */
    if (any_of(meeting_data.rooms.begin(), meeting_data.rooms.end(), mem_fn(&Room::has_Meetings)))
    {
        cout << "Cannot clear people list unless there are no meetings!" << endl;
    }
    else
    {
        clear_people_list(meeting_data.people);
        cout << all_persons_deleted_message_c << endl;
    }
}

/*
 *  Helper function that deletes the meeting pointers in rooms
 *  and clear the room list.
 */
static void clear_room_list(Room_t& rooms)
{
    /* Delete all meetings in rooms */
    for_each(rooms.begin(), rooms.end(), mem_fn(&Room::clear_Meetings));
    rooms.clear();   
}

/*
 * Called when the user enters the 'da' command.
 * Deletes all the rooms and their meetings and then
 * deletes all persons in the people list.
 * Errors: None.
 */
static void cmd_delete_all(MeetingData& meeting_data)
{
    
    clear_room_list(meeting_data.rooms);
    /* delete all individuals */
    clear_people_list(meeting_data.people);  
    cout << "All rooms and meetings deleted" << endl;
    cout << all_persons_deleted_message_c << endl; 
}

/*
 * Called when a user enters a 'sd' command.
 * Saves data by writing the people, rooms, and meetings
 * data to the named file.
 * Error: File cannot be opened for output.
 */
static void cmd_save_data(MeetingData& meeting_data)
{
    string filename;
    cin >> filename;

    ofstream outfile(filename.c_str());
    if(!outfile)
    {
        throw Error(file_cannot_open_message_c);
    }
    
    outfile << meeting_data.people.size() << endl;
    // save the data for each person into outfile
    for_each(meeting_data.people.begin(), meeting_data.people.end(), 
            bind(&Person::save, placeholders::_1, ref(outfile)));

    outfile << meeting_data.rooms.size() << endl;
    // save the data for each room into outfile
    for_each(meeting_data.rooms.begin(), meeting_data.rooms.end(),
        bind(&Room::save, placeholders::_1, ref(outfile)));

    cout << "Data saved" << endl;
    outfile.close();
}

/*
 * Roll back backed up data in the case of a failed load 
 * Function takes in four arguments: the current room and people's
 * lists and the backup room and backup people's lists.
 */
static void roll_back(Room_t& rooms, People_t& people, Room_t& rooms_backup, People_t& people_backup, ifstream& is)
{
    // clear and release resources for room and people containers
    clear_room_list(rooms);
    clear_people_list(people);
    // set room and people containers to their backups to restore state
    people = people_backup;
    rooms = rooms_backup;
    // close the file after a roll-back.
    is.close();
}

/*
 * Called when a user types an 'ld' command.
 * Restores the program state from the data in the file.
 * Errors: File cannot be opened for input, invalid data found in file.
 */
static void cmd_load_data(MeetingData& meeting_data)
{
    string filename;
    cin >> filename;

    ifstream infile(filename.c_str());
    if(!infile)
    {
        infile.close();
        throw Error(file_cannot_open_message_c);
    }
    
    int num_people;
    infile >> num_people;
    // close file
    try
    {
        file_invalid_data_check(infile);
    }
    catch(Error& e)
    {
        infile.close();
        throw;
    }
    
    // Backup copies
    Room_t rooms_backup = meeting_data.rooms;
    People_t people_backup = meeting_data.people;

    /* 
     * clear room and people lists
     * Note: We don't want to delete the people or meetings
     * because assignment to backup is shallow.
     */ 
    meeting_data.rooms.clear();
    meeting_data.people.clear();

    while (num_people-- > 0)
    {
        Person* person_to_load;
        try{
            person_to_load = new Person(infile);
        }
        // different possible exceptions thrown
        catch (...)
        {
            // roll_back closes file
            roll_back(meeting_data.rooms, meeting_data.people, rooms_backup, people_backup, infile);
            throw;
        }
        meeting_data.people.insert(person_to_load);
    }

    int num_rooms;
    infile >> num_rooms;
    try
    {
        file_invalid_data_check(infile);
    }
    catch(Error& e)
    {
        roll_back(meeting_data.rooms, meeting_data.people, rooms_backup, people_backup, infile);
        throw;
    }

    while (num_rooms-- > 0)
    {
        try{
            Room room_to_load(infile, meeting_data.people);
            auto room_it = lower_bound(meeting_data.rooms.begin(), meeting_data.rooms.end(), room_to_load);
            meeting_data.rooms.insert(room_it, room_to_load);
        }
        // different possible exceptions thrown
        catch (...)
        {
            roll_back(meeting_data.rooms, meeting_data.people, rooms_backup, people_backup, infile);
            throw;
        }
    }
    /* get rid of backup data */
    clear_room_list(rooms_backup);
    clear_people_list(people_backup);

    cout << "Data loaded" << endl;
    infile.close();
}

/*
 *  Function that handles the "qq" command.
 *  Deletes all allocated memory and prints Done.
 */
static void cmd_quit(MeetingData& meeting_data)
{
    cmd_delete_all(meeting_data);
    cout << "Done" << endl;
}
