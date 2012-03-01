#ifndef FQREADER_HPP_INCLUDED
#define FQREADER_HPP_INCLUDED

#include <string>
#include <fstream>

struct FqEntry
{
    std::string header1;
    std::string sequence;
    std::string header2;
    std::string quality;
};


class FqReader
{
    private:
        std::ifstream input_;
        std::string fileName_;
    public:
        bool open(std::string fileName);
        bool close();
        FqEntry getNextEntry();
        bool hasNextEntry();
};

#endif // FQREADER_HPP_INCLUDED
