#ifndef XMLWRITERTORRENT_HPP_INCLUDED
#define XMLWRITERTORRENT_HPP_INCLUDED

#include "BamExporter.hpp"
#include <sstream>
#include <rapidxml.hpp>
#include <rapidxml_print.hpp>

using namespace rapidxml;

class XmlWriterTorrent
{
    private:
        xml_document<> doc_;
        BamExporter* pExporter_;
    public:
        XmlWriterTorrent(BamExporter* pExporter);
        bool createXml(std::string fileName);
};


#endif // XMLWRITERTORRENT_HPP_INCLUDED
