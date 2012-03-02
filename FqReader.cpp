#include "FqReader.hpp"
#include <iostream>

//! Opens a FASTQ file.

//! Opens a FASTQ file for reading. Does not check whether file is correct or not.
//! Reports failure to std::cerr.
//!@param[in] fileName The name of the file to open.
//!@return Open operation success/failure.
bool FqReader::open(std::string fileName)
{
    fileName_ = fileName;

    //open file
    input_.open(fileName_.c_str());
    if(!input_.is_open())
    {
        std::cout << "Error: Unable to open " << fileName_ << std::endl;
        return false;
    }

    return true;
}



//! Closes FASTQ file.

//! Closes currently open FASTQ file. Doesn't care if one is open or not, so long is it is closed after the operation.
//! Reports failure to std::cerr.
//!@return Close operation success/failure.
bool FqReader::close()
{
    input_.close();
    if(input_.is_open())
    {
        std::cerr << "Error: Unable to close " << fileName_ << std::endl;
        return false;
    }

    return true;
}



//! Grabs next entry from FASTQ file.

//! User must open file first with open(). Does not check file structure is correct.
//! Reports failure to std::cerr.
//!@return Entry read from file.
FqEntry FqReader::getNextEntry()
{
    if(!input_.is_open())
    {
        std::cout << "Error: Failed to get entry from " << fileName_ << " as file is not open" << std::endl;
        return FqEntry();
    }

    FqEntry entry;
    if(!input_.eof()) getline(input_, entry.header1);
    if(!input_.eof()) getline(input_, entry.sequence);
    if(!input_.eof()) getline(input_, entry.header2);
    if(!input_.eof()) getline(input_, entry.quality);

    return entry;
};



//! Checks if there are more entries to read from open FASTQ file.

//! Essentially checks if any more lines to read.
//!@return True if has more entries.
bool FqReader::hasNextEntry()
{
    return (!input_.eof());
}
