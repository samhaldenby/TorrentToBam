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
        void createXml();


};

XmlWriterTorrent::XmlWriterTorrent(BamExporter* pExporter) : pExporter_(pExporter)
{

}


void XmlWriterTorrent::createXml()
{
    xml_node<> *node = doc_.allocate_node(node_element, "Sample");
    doc_.append_node(node);

    //for each sample
    std::map<std::string, std::string>::iterator iName = pExporter_->getOutputParser().barcode_output_name_map.begin();
    while(iName!=pExporter_->getOutputParser().barcode_output_name_map.end())
    {

        xml_node<> *name = doc_.allocate_node(node_element, iName->second.c_str());
        //output barcode info
        xml_node<> *barcode = doc_.allocate_node(node_element, "Barcode", iName->first.c_str());
        //output read count
        long readCount = pExporter_->getStats().numReadsForBarcode[iName->first];
        std::stringstream ss("");
        ss << readCount;
        std::string* countStr = new std::string("");
        ss >> *countStr;
        xml_node<> *count = doc_.allocate_node(node_element, "Read_Count", countStr->c_str());
        std::cout << iName->second << "\t" << iName->first << "\t" << readCount << "\t" << countStr->c_str() << std::endl;
        node->append_node(name);
        name->append_node(barcode);
        name->append_node(count);
        ++iName;
    }



    print(std::cout, doc_, 0);
}
#endif // XMLWRITERTORRENT_HPP_INCLUDED
