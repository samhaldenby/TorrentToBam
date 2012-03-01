#include "FqReader.hpp"
#include <iostream>
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


bool FqReader::close()
{
    input_.close();
    if(input_.is_open())
    {
        std::cout << "Error: Unable to close " << fileName_ << std::endl;
        return false;
    }

    return true;
}


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


bool FqReader::hasNextEntry()
{
    return (!input_.eof());
}
