#ifndef PERSON_H
#define PERSON_H

#include "Utility.h"
#include <fstream>
#include <string>
#include <map>
#include <set>

class Meeting;

class Person {
public:
    Person(const std::string& firstname_, const std::string& lastname_, const std::string& phoneno_) :
        firstname(firstname_),
        lastname(lastname_),
        phoneno(phoneno_) {}
    // construct a Person object with only a lastname
    Person(const std::string& lastname_) : lastname(lastname_) {}

    /* *** Disallow all forms of copy/move construction or assignment */
    // These declarations help ensure that Person objects are unique,
    // like they are in the problem domain
    Person(const Person& original) = delete;
    Person(Person&& original) = delete;
    Person& operator= (const Person& rhs) = delete;
    Person& operator= (Person&& rhs) = delete;

    // Construct a Person object from a file stream in save format.
    // Throw Error exception if invalid data discovered in file.
    // No check made for whether the Person already exists or not.
    // Input for a member variable value is read directly into the member variable.
    Person(std::ifstream& is);
    
    // Accessors
    std::string get_lastname() const
        { return lastname; }
    
    // Write a Person's data to a stream in save format with final endl.
    void save(std::ostream& os) const;

    // Adds a commtiment to the person given a room number and meeting ptr.
    // Throws an error if there is a commitment conflict.
    void add_commitment(int room_number, const Meeting* meeting);

    // Removes a commitment from a Person given the room number and 
    // meeting time. Returns true if a commitment was removed.
    // This is to allow us to find out if we have to add a new commitment
    // during a reschedule.
    bool remove_commitment(int room_number, int meeting_time);

    // prints the commitments for this Person.
    void print_commitment() const;

    // removes all commitments for this person for the given room number.
    void remove_room_commitments(int room_number);

    // clears all the commitments for this person.
    void clear_Commitments()
        {commitments.clear();}

    // Function that checks for a commitment conflict given a meeting time.
    // Returns true if a commitment conflict was found and false if it was not found.
    bool has_commitment_conflict(int time) const;

    // This operator defines the order relation between Persons, based just on the last name
    bool operator< (const Person& rhs) const
        { return lastname < rhs.lastname;  }

    // friend declaration for output operator for reference to Person object.
    friend std::ostream& operator<< (std::ostream& os, const Person& person);
    // friend declaration for output operator for pointer to Person object.
    friend std::ostream& operator<< (std::ostream& os, const Person* person);

private:
    std::string firstname;
    std::string lastname;
    std::string phoneno;
    // map of integers(representing room numbers) to vectors of meetings
    // corresponding to meetings for the key value
    // map is ordered by room number
    using Commitments_t = std::map<int, std::set<const Meeting*, Less_than_ptr<const Meeting*>>>;
    Commitments_t commitments;
};

// output firstname, lastname, phoneno with one separating space, NO endl
std::ostream& operator<< (std::ostream& os, const Person& person);

// output firstname, lastname, phoneno with one separating space, NO endl
std::ostream& operator<< (std::ostream& os, const Person* person);

#endif
