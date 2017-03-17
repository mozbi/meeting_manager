#ifndef UTILITY_H
#define UTILITY_H

#include <fstream>
#include <set>

#define NDBUG
 
/* Utility functions, constants, and classes used by more than one other modules */

class Person;
 
const char* const invalid_file_data_message_c = "Invalid data found in file!";
const char* const no_meeting_at_time_message_c = "No meeting at that time!";
const char* const meeting_exists_at_time_message_c =  "There is already a meeting at that time!";

 

// function object class template that compares
// two pointers using *T's operator<
template<typename T>
struct Less_than_ptr
{
    bool operator()(const T p1, const T p2) const {return *p1 < *p2;}
};   

// a simple class for error exceptions - msg points to a C-string error message
struct Error {
	Error(const char* msg_ = "") :
		msg(msg_)
		{}
	const char* msg;
};
 
// alias for set of people.
using People_t = std::set<Person*, Less_than_ptr<const Person*>>;

// function that checks if the ifstream is in a good
// state after reading from a file.
// Throws an error if it is not in a good state.
void file_invalid_data_check(std::ifstream& is);

#endif
