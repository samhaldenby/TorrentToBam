#include <iostream>
#include "Opts.hpp"
#include <api/BamReader.h>
#include <api/BamWriter.h>
#include <boost/xpressive/xpressive.hpp>

#include "FqReader.hpp"
#include "BamExporter.hpp"
#include "XmlWriterTorrent.hpp"

//naughty globals!
const int DEFAULT_BARCODE_LENGTH = 4;
const int DEFAULT_MIN_READ_LENGTH = 20;

int regexCounter = 0;
int regexMatch=0;
int regexMismatch=0;
int regexMatchIfBarcodeIncluded=0;

using namespace boost::xpressive;

//struct OutputParser
//{
//    std::map<std::string, BamTools::BamWriter*> barcode_file_map;
//    std::map<std::string, std::string> barcode_output_name_map;
//} outputParser;


std::map<std::string, int> bcMap;








std::string getRunVersionInfo(std::string fileName)
{
    std::ifstream input;
    input.open(fileName.c_str());
    if(!input.is_open())
    {
        std::cerr << "Error: Failed to open version file '" << fileName << "'" << std::endl;
        return "";
    }

    std::string retStr = "@PG\tID:Version_Information\tPN:TorrentToBam\tVN:0.0.1\tDS:";
    std::string line("");
    while(!input.eof())
    {
        getline(input, line);
        if(line.size()>0)
        {
            retStr+=line;
            retStr+=":";
        }
    }

    retStr = retStr.substr(0,retStr.size()-1) + "\n";

    return retStr;
}



void displayHelp()
{
    std::cout << "*** TorrentToBam ***" << std::endl;
    std::cout << "Function: Takes a fastq file and generates a bam file" << std::endl;
    std::cout << "Usage : ./TorrentToBam -i in.fq -o output_directory -r runId -s sampleSheet.txt -v versionFile.txt" << std::endl;
    std::cout << "Options: -b     Barcode length [" << DEFAULT_BARCODE_LENGTH << "]" << std::endl;
    std::cout << "         -m     Min read length after barcode and tag removal [" << DEFAULT_MIN_READ_LENGTH << "]" << std::endl;
    std::cout << std::endl;
    std::cout << "Notes: sample sheet must be two column tab-seperated (sample barcode) with a header line" << std::endl;
}

int main(int argc, char**argv)
{
    Opts opts = getOpts(argc, argv);
    std::string fqInName("");
    std::string runId("");
    std::string outputDirectory("");
    std::string sampleSheetName("");
    std::string versionFileName("");
    int barcodeLength;
    int minReadLength;
    fqInName=opts["-i"];
    outputDirectory=opts["-o"];
    runId=opts["-r"];
    versionFileName=opts["-v"];
    sampleSheetName = opts["-s"];


    opts["-b"]!="" ? barcodeLength = atoi(opts["-b"].c_str()) : barcodeLength = barcodeLength;
    opts["-m"]!="" ? minReadLength = atoi(opts["-m"].c_str()) : minReadLength = minReadLength;


    if(fqInName=="" || outputDirectory=="" || sampleSheetName=="" || runId=="")
    {
        displayHelp();
        return 0;
    }


    //restructure command line for putting in bam header
    std::string cmd="@PG\tID:Fastq_to_bam\tPN:TorrentToBam\tDS:Converts FQ files to BAM files and removes tags and barcodes\tVN:0.0.1\tCL:";
    for(int a=0; a< argc; ++a)
    {
        cmd+=argv[a];
        cmd+=" ";
    }
    cmd+="\n";



    //create header
    std::string header = "@HD\tVN:1.0\tSO:unsorted\n";
    //add command line to header
    header+=cmd;

    //add run id
    std::string runHeader = "@RG\tID:" + runId + "\tCN:EASIH\tPL:IONTORRENT\tSM:" + runId + "\n";
    header+=runHeader;
    //if version file provided, add to header
    if(versionFileName!="")
    {
        header+=getRunVersionInfo(versionFileName);
    }

    BamTools::RefVector refs = BamTools::RefVector();

    BamExporter bamExporter(DEFAULT_BARCODE_LENGTH, DEFAULT_MIN_READ_LENGTH);
    bamExporter.setOutputDir(outputDirectory);
    bamExporter.setBarcodeLength(barcodeLength);
    bamExporter.setMinReadLength(minReadLength);
    bamExporter.setHeader(header);
    bamExporter.setRefs(refs);
    if(!bamExporter.readSampleSheet(sampleSheetName))
    {
        std::cerr << "ERROR: Failed to read sample sheet '" << sampleSheetName << "'" << std::endl;
        return 1;
    }




    FqReader fqReader;
    fqReader.open(fqInName);

    FqEntry entry;

    unsigned long readCounter = 0;
    while(fqReader.hasNextEntry())
    {
//        entry = fqReader.getNextEntry();
        if(bamExporter.exportAlignment(fqReader.getNextEntry()))
        {
            if (++readCounter%10000==0)
            {
                std::cout << "\r";
                bamExporter.getStats().report();
//                std::cout << "\rReads parsed: " << readCounter << "\t" << regexMatch << "\t" << regexMismatch << "\t" << regexMatchIfBarcodeIncluded;
                std::cout.flush();
            }
        }
    }

    //calculate final stats
    bamExporter.getStats().generateFinalStats();

    std::cout << "\r";
    bamExporter.getStats().report();
    std::cout << std::endl;
    bamExporter.getStats().reportBarcodeFrequencies();
    bamExporter.getStats().reportReadLengthStats();


    //create xml output
    XmlWriterTorrent xmlWriter(&bamExporter);
    std::string xmlName = outputDirectory + "/" + runId + "_runStatistics.xml";

    if(!xmlWriter.createXml(xmlName.c_str()))
    {
        std::cerr << "ERROR\t:\tFailed to open xml output file '" << xmlName << "'" << std::endl;
        return 1;
    }

    //clean up
    fqReader.close();
    bamExporter.closeFiles();




    return 0;
}
