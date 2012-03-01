#include <iostream>
#include "Opts.hpp"
#include <api/BamReader.h>
#include <api/BamWriter.h>
#include <boost/xpressive/xpressive.hpp>

#include "FqReader.hpp"

//naughty globals!
int barcode_length = 4;
int min_read_length = 20;
BamTools::RefVector refs = BamTools::RefVector();
std::string header="";
int regexCounter = 0;
int regexMatch=0;
int regexMismatch=0;
int regexMatchIfBarcodeIncluded=0;

using namespace boost::xpressive;

struct OutputParser
{
    std::map<std::string, BamTools::BamWriter*> barcode_file_map;
    std::map<std::string, std::string> barcode_output_name_map;
} outputParser;


std::map<std::string, int> bcMap;




//BamTools::BamAlignment createBamEntryFromFqEntry(FqEntry entry)
//{
////    std::cout << entry.sequence.size() << "\t" << entry.quality.size() << "\t(" << min_read_length << "," << barcode_length << ")" << std::endl;
//    BamTools::BamAlignment bamAl;
//    bamAl.Qualities = entry.quality.substr(barcode_length);
//    bamAl.QueryBases = entry.sequence.substr(barcode_length);
//    bamAl.Name = entry.header1;
//    bcMap[entry.sequence.substr(0,barcode_length)]+=1;
//    bamAl.SetIsMapped(false);
//    bamAl.AddTag<std::string>("BC","Z",entry.sequence.substr(0,barcode_length));
//    bamAl.AddTag<std::string>("QT","Z",entry.quality.substr(0,barcode_length));
//    bamAl.Length = entry.sequence.size();
//    return bamAl;
//}


void exportAlignment(FqEntry entry)
{
    BamTools::BamAlignment bamAl;

    sregex rex = bol >> "TGTA" >> _w >> *(~_n) >> "CAGT";
    smatch what;
    std::string query = entry.sequence.substr(barcode_length);

    //does regex hit query?
    if(regex_search(query.begin(), query.end(), what, rex))
    {
//        std::cout << "size: " << what.size() << "\t" << what.length(0) << "\t" << query.size() << std::endl;
//        std::cout << "TRUE: (" << what[1] << ")\t" << what[0] << std::endl;
//        std::cout << what.length(0) << std::endl;
        //only remove if a sensible length (i.e. <30), otherwise it could span 100s of bases (which is unlikely to be tag!)
        if(what.length(0) < 30)
        {
//            std::cout << query << "\t" << query.substr(what.length(0)) << std::endl;
            query = query.substr(what.length(0));
        }

        regexMatch+=1;
//            int dump; std::cin >> dump;

    }
    //if not, try against sequence before removing barcode (no real reason why - just for debugging);
    else
    {
//        std::cout << "FALSE: " << query << std::endl;
        query = entry.sequence;
        if(regex_search(query.begin(), query.end(), what, rex))
        {
    //        std::cout << "size: " << what.size() << "\t" << what.length(0) << "\t" << query.size() << std::endl;
//            std::cout << "FULL****: (" << what[1] << ")\t" << what[0] << std::endl;
            regexMatchIfBarcodeIncluded+=1;
//                int dump; std::cin >> dump;

        }

        regexMismatch+=1;
    }

    bamAl.Qualities = entry.quality.substr(barcode_length);
    bamAl.QueryBases = entry.sequence.substr(barcode_length);
    bamAl.Name = entry.header1;
    bamAl.SetIsMapped(false);
    std::string barcode = entry.sequence.substr(0,barcode_length);
    bamAl.AddTag<std::string>("BC","Z",barcode);
    bamAl.AddTag<std::string>("QT","Z",entry.quality.substr(0,barcode_length));
    bamAl.Length = entry.sequence.size();

    //export
    std::map<std::string, BamTools::BamWriter*>::iterator i = outputParser.barcode_file_map.find(barcode);
    if(i==outputParser.barcode_file_map.end())
    {
        outputParser.barcode_file_map["NM"]->SaveAlignment(bamAl);
    }
    else
    {
        i->second->SaveAlignment(bamAl);
    }


}

OutputParser readSampleSheet(std::string sampleSheetName)
{
    OutputParser op;
    std::ifstream input;
    input.open(sampleSheetName.c_str());
    if(!input.is_open())
    {
        std::cout << "Error: Failed to open " << sampleSheetName << std::endl;
        return op;
    }

    //remove header line
    std::string dump(""), name(""), barcode("");
    input >> dump >> dump;
    while(input >> name >> barcode)
    {
        if(name!="" && barcode!="")
        {
            //double check barcodes are correct length etc
            if(barcode.size()!=barcode_length)
            {
                std::cout << "Warning: Barcode size in sample sheet does not match that specified on the command line (" << barcode.size() << " vs " <<  barcode_length << ")." << std::endl;
                std::cout << "Resizing desired barcode length to " << barcode.size() << std::endl;
                barcode_length = barcode.size();
            }
            op.barcode_output_name_map[barcode]=name;
            BamTools::BamWriter* pO = new BamTools::BamWriter();
            std::string fileName = name + "_" + barcode + ".bam";
            pO->Open(fileName, header, refs);
            op.barcode_file_map[barcode]=pO;

        }
        barcode="";
        name="";
    }

    //add non-match stuff
    std::string fileName = "NonMatched.bam";
    BamTools::BamWriter* pO = new BamTools::BamWriter();
    pO->Open(fileName, header, refs);
    op.barcode_file_map["NM"]=pO;

    return op;

}

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
    std::cout << "Usage : ./TorrentToBam -i in.fq -o out.bam -s sampleSheet.txt -v versionFile.txt" << std::endl;
    std::cout << "Options: -b     Barcode length [4]" << std::endl;
    std::cout << "         -m     Min read length after barcode removal [20]" << std::endl;
    std::cout << std::endl;
    std::cout << "Notes: sample sheet must be two column tab-seperated (sample barcode) with a header line" << std::endl;
}

int main(int argc, char**argv)
{
    Opts opts = getOpts(argc, argv);
    std::string fqInName("");
    std::string bamOutName("");
    std::string sampleSheetName("");
    std::string versionFileName("");

    fqInName=opts["-i"];
    bamOutName=opts["-o"];
    versionFileName=opts["-v"];
    sampleSheetName = opts["-s"];

    opts["-b"]!="" ? barcode_length = atoi(opts["-b"].c_str()) : barcode_length = barcode_length;
    opts["-m"]!="" ? min_read_length = atoi(opts["-m"].c_str()) : min_read_length =min_read_length;


    if(fqInName=="" || bamOutName=="" || sampleSheetName=="")
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




    //create output bam file
    header = "@HD\tVN:1.0\tSO:unsorted\n";
    //add command line to header
    header+=cmd;

    //if version file provided, add to header
    if(versionFileName!="")
    {
        header+=getRunVersionInfo(versionFileName);
    }

//     int dump;
//    std::cin >> dump;
//    std::cout << header << std::endl;
//
//    std::cin >> dump;
    BamTools::RefVector refs = BamTools::RefVector();

    //read sample sheet
    outputParser = readSampleSheet(sampleSheetName);


//    BamTools::BamWriter writer;
//    writer.Open(bamOutName, header, refs);

    FqReader fqReader;
    fqReader.open(fqInName);

    FqEntry entry;

    unsigned long readCounter = 0;
    while(fqReader.hasNextEntry())
    {
        entry = fqReader.getNextEntry();

        //only export if sequence > min_read_length (taking into account barcode);
        if(entry.sequence.size() > min_read_length + barcode_length)
        {
            exportAlignment(entry);
//            writer.SaveAlignment(createBamEntryFromFqEntry(entry));
        }

        if (++readCounter%10000==0)
        {
            std::cout << "\rReads parsed: " << readCounter << "\t" << regexMatch << "\t" << regexMismatch << "\t" << regexMatchIfBarcodeIncluded;
            std::cout.flush();
        }
    }

    std::cout << "\rReads parsed: " << readCounter << std::endl;


    //clean up
    fqReader.close();
    std::map<std::string, BamTools::BamWriter*>::iterator iO = outputParser.barcode_file_map.begin();
    while(iO!=outputParser.barcode_file_map.end())
    {
        iO->second->Close();
        ++iO;
    }



    return 0;
}
