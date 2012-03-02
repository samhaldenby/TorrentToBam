#ifndef FQREADER_HPP_INCLUDED
#define FQREADER_HPP_INCLUDED

#include <string>
#include <fstream>

struct FqEntry
{
    std::string header1;    //!< First header line beginning with '@'
    std::string sequence;   //!< Sequence line
    std::string header2;    //!< Second header line beginning with '+'
    std::string quality;    //!< Quality score line
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
