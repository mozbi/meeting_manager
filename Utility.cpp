#include "Utility.h"

using std::ifstream;

void file_invalid_data_check(ifstream& is)
{
    if(!is.good())
    {
        throw Error(invalid_file_data_message_c);
    }
}
