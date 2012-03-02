#include "XmlWriterTorrent.hpp"

//! Constructor

//! Sets pointer to BamExporter so that grabbing stats from it is possible
//!@param[in] pExporter Pointer to BamExporter which contains stats bundle
XmlWriterTorrent::XmlWriterTorrent(BamExporter* pExporter) : pExporter_(pExporter)
{

}


//! Writes run statistics to XML file

//! Exports per-barcode information for the run. i.e. sample name, number of reads, number of bases, longest/shortest/average read length
//!@param[in] fileName Name of file to export to
//!@return File open operation success/failure.
bool XmlWriterTorrent::createXml(std::string fileName)
{
    // TODO (sh695#1#): Needs deletion of 'new'd pointer data to avoid memory leak. Low priority: Only called at end of program life
    //create doc and root node
    xml_node<> *node = doc_.allocate_node(node_element, "Sample");
    doc_.append_node(node);

    //for each sample
    std::map<std::string, std::string>::iterator iName = pExporter_->getOutputParser().barcode_output_name_map.begin();
    while(iName!=pExporter_->getOutputParser().barcode_output_name_map.end())
    {
        //generate name node
        xml_node<> *name = doc_.allocate_node(node_element, iName->second.c_str());

        //generate barcode node
        xml_node<> *barcode = doc_.allocate_node(node_element, "Barcode", iName->first.c_str());

        //generate read count
        long readCount = pExporter_->getStats().numReadsForBarcode[iName->first];
        std::stringstream ss("");
        ss << readCount;
        std::string* countStr = new std::string("");
        ss >> *countStr;
        xml_node<> *count = doc_.allocate_node(node_element, "Read_Count", countStr->c_str());

        //generate read length info
        long totalBases = pExporter_->getStats().totalBasesForBarcode[iName->first];
        int shortest = pExporter_->getStats().shortestReadLenForBarcode[iName->first];
        int longest = pExporter_->getStats().longestReadLenForBarcode[iName->first];
        int average = pExporter_->getStats().averageReadLenForBarcode[iName->first];
        std::cout << iName->second << "\t" << iName->first << "\t" << totalBases << "\t" << shortest << "\t" << longest << "\t" << average << std::endl;
        std::stringstream totSs("");
        std::stringstream shSs("");
        std::stringstream loSs("");
        std::stringstream avSs("");
        std::string* totStr = new std::string("");
        std::string* shStr = new std::string("");
        std::string* loStr = new std::string("");
        std::string* avStr = new std::string("");
        totSs << totalBases;
        shSs << shortest;
        loSs << longest;
        avSs << average;
        totSs >> *totStr;
        shSs >> *shStr;
        loSs >> *loStr;
        avSs >> *avStr;
        xml_node<> *totNode = doc_.allocate_node(node_element, "Total_Bases", totStr->c_str());
        xml_node<> *shortNode = doc_.allocate_node(node_element, "Shortest_Read_Length", shStr->c_str());
        xml_node<> *longNode = doc_.allocate_node(node_element, "Longest_Read_Length", loStr->c_str());
        xml_node<> *aveNode = doc_.allocate_node(node_element, "Average_Read_Length", avStr->c_str());

        //stick tree together
        node->append_node(name);
        name->append_node(barcode);
        name->append_node(count);
        name->append_node(totNode);
        name->append_node(shortNode);
        name->append_node(longNode);
        name->append_node(aveNode);

        ++iName;
    }


    //output
    std::ofstream output;
    output.open(fileName.c_str());
    if(!output.is_open())
    {
        return false;
    }
    output << doc_;
    output.close();

    return true;
}
