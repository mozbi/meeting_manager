#include "String.h"
#include <cstring>
#include <utility>
#include <iostream>
#include <new>

char String::a_null_byte = '\0';
int String::number = 0;
int String::total_allocation = 0;
bool String::messages_wanted = false;

// Default initialization is to contain an empty string with no allocation.
// If a non-empty C-string is supplied, this String gets minimum allocation.
String::String(const char* cstr_) : sz(strlen(cstr_))
{
    if (messages_wanted)
    {
        std::cout << "Ctor: \"" << cstr_ << "\"" << std::endl;
    }
    if (!sz)
    {
        str_ptr = &a_null_byte;
        allocation = 0;
    }
    else
    {
        try{
            str_ptr = new char[sz+1];
        }
        catch(std::bad_alloc& ba)
        {
            throw;
        }
        strcpy(str_ptr, cstr_);
        /* update static variable */
        allocation = sz+1;
        total_allocation += allocation;
    }
    ++number;
}

// The copy constructor initializes this String with the original's data,
// and results in a minimum allocation.
String::String(const String& original) : sz(original.sz) 
{
    if (messages_wanted)
    {
        std::cout << "Copy ctor: \"" << original << "\"" << std::endl;
    }
    if (!sz)
    {
        str_ptr = &a_null_byte;
        allocation = 0;
    }
    else
    {
        /* allocate then modify allocation. */
        try{
            str_ptr = new char[sz+1];
        }
        catch(std::bad_alloc& ba)
        {
            throw;
        }
        strcpy(str_ptr, original.str_ptr);
        allocation = sz+1;
         /* update static variable */
        total_allocation += allocation;
    }
    ++number;
}
// Move constructor - take original's data, replace it with values like 
// those of a default constructed objeString::String(String&& original) : sz(original.sz), str_ptr(original.str_ptr) noexcept
String::String(String&& original) noexcept : 
sz(original.sz), allocation(original.allocation), str_ptr(original.str_ptr)
{
    if (messages_wanted)
    {
        std::cout << "Move ctor: \"" << original << "\"" << std::endl;
    }
    original.sz = 0;
    original.allocation = 0;
    original.str_ptr = nullptr;
     /* update static variable */
    ++number;
}

// deallocate C-string memory
String::~String() noexcept
{
    if (messages_wanted)
    {
        std::cout << "Dtor: \"";
        for (int i=0; i < sz; ++i)
        {
            std::cout << str_ptr[i];
        }
        std::cout << "\"" << std::endl;
    }

    if(str_ptr != &a_null_byte)
    {
        delete[] str_ptr;
    }
    --number;
    total_allocation -= allocation;
}
	
// Assignment operators
// Left-hand side gets a copy of rhs data and gets minimum allocation.
// This operator use the copy-swap idiom for assignment.
String& String::operator= (const String& rhs)
{
    if(messages_wanted)
    {
        std::cout << "Copy assign from String:  \"" << rhs << "\"" << std::endl;
    }

    String temp_rhs(rhs);
    swap(temp_rhs);
    return *this;
}

// This operator creates a temporary String object from the rhs C-string, and swaps the contents
String& String::operator= (const char* rhs)
{
    if(messages_wanted)
    {
        std::cout << "Assign from C-string:  \"" << rhs << "\"" << std::endl;
    }

    // check logic and whether this can be added into a function.
    String temp_rhs(rhs);
    swap(temp_rhs);
    return *this;
}

// Move assignment - simply swaps contents with rhs without any new allocation or copying
String& String::operator= (String&& rhs) noexcept
{
    if(messages_wanted)
    {
        std::cout << "Move assign from String:  \"" << rhs  << "\"" << std::endl;
    }
    swap(rhs);
    /* Total allocation change? */
    return *this;
}

// Return a reference to character i in the string.
// Throw exception if 0 <= i < size is false.
char& String::operator[] (int i)
{
    // put in function.
    if ( i < 0 || i > sz - 1) 
    {
     //throw a bad-subscript exception
        throw String_exception("String index out of range");
    }
    return str_ptr[i];
}

const char& String::operator[] (int i) const	// const version for const Strings
{
    if ( i < 0 || i > sz - 1) 
    {
                // throw a bad-subscript exception
        throw String_exception("String index out of range");
    }
    return str_ptr[i];
}

// Modifiers
// Set to an empty string with minimum allocation by create/swap with an empty string.
void String::clear()
{
    // check to make sure this is right.
    String temp = String();
    swap(temp);
}

// operations that involve adding characters use a doubling rule for allocation.
/* These concatenation operators add the rhs string data to the lhs object.
They do not create any temporary String objects. They either directly copy the rhs data
into the lhs space if it is big enough to hold the rhs, or allocate new space
and copy the old lhs data into it followed by the rhs data. The lhs object retains the
final memory allocation. */
// If the char rhs is a null byte ('\0') the effects are undefined.
void String::resize_if_full(int size_to_append)
{
    int minimum_alloc_required = sz + 1 + size_to_append;
    if (allocation < minimum_alloc_required)
    {
        /* might have to be moved down. */
        int old_alloc = allocation;
        /* new then modify allocation. */
        char* bigger_str_ptr;
        try{
            bigger_str_ptr = new char[2*(minimum_alloc_required)];
        }
        catch(std::bad_alloc& ba)
        {
            throw;
        }
        strcpy(bigger_str_ptr, str_ptr);
        allocation = 2*(minimum_alloc_required); 
        if (str_ptr != &a_null_byte)
            delete[] str_ptr;
        str_ptr = bigger_str_ptr;
        total_allocation += (allocation - old_alloc); 
    }
}

String& String::operator += (char rhs)
{
    /* we have run out of space:
     * allocation == size+1 so if it's smaller
     * than size+2, it means we have run out of
     * allocated space.
     */
    /* length of character is 1 */
    resize_if_full(1);
    str_ptr[sz] = rhs;
    str_ptr[++sz] = '\0';
    return *this;

}

// For the following, if the rhs is empty, no change is made to lhs String.
String& String::operator += (const char* rhs)
{
    if (!rhs)
    {
        return *this;
    }
    resize_if_full(strlen(rhs));
    sz += strlen(rhs);
    strcat(str_ptr, rhs);
    return *this;

}
String& String::operator += (const String& rhs)
{
    if (!rhs.sz)
    {
        return *this;
    }
    resize_if_full(rhs.sz);
    *this += rhs.str_ptr;
    return *this;
}

/* Swap the contents of this String with another one.
The member variable values are interchanged, along with the
pointers to the allocated C-strings, but the two C-strings
are neither copied nor modified. No memory allocation/deallocation is done. */
void String::swap(String& other) noexcept
{
    std::swap(sz, other.sz);
    std::swap(allocation, other.allocation);
    std::swap(str_ptr, other.str_ptr);
}
	
// compare lhs and rhs strings; constructor will convert a C-string literal to a String.
// comparison is based on std::strcmp result compared to 0
bool operator== (const String& lhs, const String& rhs)
{
    if(lhs.size() != rhs.size())
    {
        return false;
    }
    return !strcmp(lhs.c_str(), rhs.c_str());
}
bool operator!= (const String& lhs, const String& rhs)
{
    return !(lhs==rhs);
}

bool operator< (const String& lhs, const String& rhs)
{
    return (strcmp(lhs.c_str(), rhs.c_str()) < 0);
}
bool operator> (const String& lhs, const String& rhs)
{
    return (strcmp(lhs.c_str(), rhs.c_str()) > 0);
}

/* Concatenate a String with another String.
 If one of the arguments is a C-string, the compiler will automatically call the String constructor
 to create a temporary String from it in order match this function (inefficient, but instructive).
 This automatic behavior would be disabled if the String constructor was declared "explicit".
 This function constructs a copy of the lhs in a local String variable, 
 then concatenates the rhs to it with operator +=, and returns it. */
String operator+ (const String& lhs, const String& rhs)
{
    String temp_lhs(lhs);
    /* check this: cannot return temp_lhs+=rhs */
    temp_lhs += rhs;
    return temp_lhs;
}

// Input and output operators
// The output operator writes the contents of the String to the stream
std::ostream& operator<< (std::ostream& os, const String& str)
{
    for (int i=0; i<str.size(); ++i)
    {
        os << str[i];
    }
    return os;
}

/* The input operator clears the supplied String, then starts reading the stream.
It skips initial whitespace, then concatenates characters into
the supplied str until whitespace is encountered again. The terminating
whitespace remains in the input stream, analogous to how string input normally works.
str will get expanded as needed, and retains the final allocation.
If the input stream fails, str contains whatever characters were read. */
std::istream& operator>> (std::istream& is, String& str)
{
    str.clear();
    is >> std::ws;
    char ch ='\0';
    while(!isspace(is.peek()))
    {
        is.get(ch);
        str+=ch;
        if (!is)
        {
            return is;
        }
    }
    return is;
}

