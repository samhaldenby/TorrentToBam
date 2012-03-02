#ifndef BAMEXPORTER_HPP_INCLUDED
#define BAMEXPORTER_HPP_INCLUDED

#include <boost/xpressive/xpressive.hpp>
#include <api/BamWriter.h>
#include <api/BamReader.h>
#include "FqReader.hpp"
#include "TorrentReadStatsBundle.hpp"

struct OutputParser
{
    std::map<std::string, BamTools::BamWriter*> barcode_file_map;   //!< Map of barcodes => file handles
    std::map<std::string, std::string> barcode_output_name_map;     //!< Map of barcodes => file names
};

class BamExporter
{
    private:
        OutputParser outputParser_;
        TorrentReadStatsBundle stats_;
        int barcodeLength_;
        int minReadLength_;
        std::string outputDir_;
        std::string header_;
        BamTools::RefVector refs_;
        boost::xpressive::sregex rex_;

    public:
        BamExporter(int barcodeLength, int minReadLength);
        void setHeader(std::string header);
        void setRefs(BamTools::RefVector refs);
        void setOutputDir(std::string outputDir);
        bool setBarcodeLength(int barcodeLength);
        bool setMinReadLength(int minReadLength);
        bool readSampleSheet(std::string sampleSheetName);
        bool exportAlignment(FqEntry entry);
        void closeFiles();
        TorrentReadStatsBundle& getStats();
        OutputParser& getOutputParser();
};



#endif // BAMEXPORTER_HPP_INCLUDED
