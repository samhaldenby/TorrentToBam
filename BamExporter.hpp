#ifndef BAMEXPORTER_HPP_INCLUDED
#define BAMEXPORTER_HPP_INCLUDED

#include <boost/xpressive/xpressive.hpp>
#include <api/BamWriter.h>
#include <api/BamReader.h>
#include "FqReader.hpp"
#include "TorrentReadStatsBundle.hpp"

struct OutputParser
{
    std::map<std::string, BamTools::BamWriter*> barcode_file_map;
    std::map<std::string, std::string> barcode_output_name_map;
};

class BamExporter
{
    private:
        OutputParser outputParser_;
        TorrentReadStatsBundle stats_;
        int barcodeLength_;
        int minReadLength_;
        std::string header_;
        BamTools::RefVector refs_;
        boost::xpressive::sregex rex_;

    public:
        BamExporter(int barcodeLength, int minReadLength);
        bool setHeader(std::string header);
        bool setRefs(BamTools::RefVector refs);
        bool setBarcodeLength(int barcodeLength);
        bool setMinReadLength(int minReadLength);
        bool readSampleSheet(std::string sampleSheetName);
        bool exportAlignment(FqEntry entry);
        void closeFiles();
        TorrentReadStatsBundle& getStats();
        OutputParser& getOutputParser();
};



#endif // BAMEXPORTER_HPP_INCLUDED
