#include "BamExporter.hpp"

//! Constructor

//! Creates Exporter class and initialises barcode length and minimum acceptable read length (post tag/barcode trimming).
//! Sets regular expression pattern for tag searching.
//!@param[in] barcodeLength Length of barcode sequence.
//!@param[in] minReadLength Minimum acceptable read length, post trimming.
BamExporter::BamExporter(int barcodeLength, int minReadLength) : barcodeLength_(barcodeLength), minReadLength_(minReadLength)
{
    //set barcode regex
    rex_ = boost::xpressive::bol >> "TGTA" >> boost::xpressive::_w >> *(~boost::xpressive::_n) >> "CAGT";
}



//! Changes barcode length

//! Checks lower but not upper bounds, i.e. must be greater than 0
//!@param[in] barcodeLength Length of barcode sequence.
//!@return Operation success/failure
bool BamExporter::setBarcodeLength(int barcodeLength)
{
    if(barcodeLength <= 0)
    {
        std::cerr << "Warning: Invalid barcode length selected (" << barcodeLength << "). Barcode length will stay as " << barcodeLength_ << std::endl;
        return false;
    }

    barcodeLength_ = barcodeLength;
    std::cout << "Barcode length set to " << barcodeLength << std::endl;
    return true;
}




//! Changes minimum acceptable read length, post trimming

//! Checks lower but not upper bounds, i.e. must be greater than 0
//!@param[in] minReadLength Minimum acceptable read length.
//!@return Operation success/failure
bool BamExporter::setMinReadLength(int minReadLength)
{
    if(minReadLength <= 0)
    {
        return false;
    }

    minReadLength_ = minReadLength;
    std::cout << "Minimum read length set to " << minReadLength << std::endl;
    return true;
}



//! Set BAM header for output files

//! User must format header string according to SAM specifications
//!@param[in] header Formatted header.
void BamExporter::setHeader(std::string header)
{
    header_ = header;
}



//! Set references that BAM file was aligned to

//! Only required as BamTools API requires this information despite using unaligned BAM file output.
//! Therefore, this is usually an empty vector.
//!@param[in] refs References (usually empty RefVector)
void BamExporter::setRefs(BamTools::RefVector refs)
{
    refs_ = refs;
}



//! Set directory for writing output files

//! Directory must already exist. This is carried out by wrapper script
//!@param[in] outputDir Project output directory path.
void BamExporter::setOutputDir(std::string outputDir)
{
    outputDir_ = outputDir;
}

//! Reads sample sheet information

//! Extracts information from sample sheet. Sample sheet must be tab-seperated with headers
//! Columns are 'Sample Name' and 'Barcode Sequence'.
//! Prepares output files based on this information, that is stored in internal OutputParser class
//!@param[in] sampleSheetName File name of sample sheet
//!@return Operation success/failure
bool BamExporter::readSampleSheet(std::string sampleSheetName)
{
    std::ifstream input;
    input.open(sampleSheetName.c_str());
    if(!input.is_open())
    {
        std::cout << "Error: Failed to open " << sampleSheetName << std::endl;
        return false;
    }

    //remove header line
    std::string dump(""), name(""), barcode("");
    input >> dump >> dump;
    while(input >> name >> barcode)
    {
        if(name!="" && barcode!="")
        {
            //double check barcodes are correct length etc
            if(barcode.size()!=barcodeLength_)
            {
                std::cerr << "Warning: Barcode size in sample sheet does not match that specified on the command line (" << barcode.size() << " vs " <<  barcodeLength_ << ")." << std::endl;
                std::cerr << "       : Setting desired barcode length to " << barcode.size() << std::endl;
                barcodeLength_ = barcode.size();
            }
            outputParser_.barcode_output_name_map[barcode]=name;
            BamTools::BamWriter* pO = new BamTools::BamWriter();
            std::string fileName = outputDir_ + "/" + name + ".bam";
            pO->Open(fileName, header_, refs_);
            outputParser_.barcode_file_map[barcode]=pO;

        }
        barcode="";
        name="";
    }

    //add non-match stuff
    std::string fileName = outputDir_ + "/NonMatched.bam";
    BamTools::BamWriter* pO = new BamTools::BamWriter();
    pO->Open(fileName, header_, refs_);
    outputParser_.barcode_file_map["NM"]=pO;
    outputParser_.barcode_output_name_map["NM"]="Non_Matched";

    //pass info to stats bundle
//    stats_.initBarcodes(outputParser_.barcode_output_name_map);

    return true;

}


//! Writes valid reads to correct BAM file

//! Determines (1) which file to write to based on barcode sequence,
//! (2) removes any tag sequences from read, along with barcode,
//! (3) writes to correct BAM file if read is sufficiently long.
//! Also keeps track of run statistics.
//!@param[in] entry FASTQ entry
//!@return True if read was long enough to export, false if not.
bool BamExporter::exportAlignment(FqEntry entry)
{
    //iterate stats counters
    stats_.numReads+=1;

    //early escape check if read definitely too short
    if(entry.sequence.size() < minReadLength_ + barcodeLength_)
    {
        stats_.numTooShortReadsBeforeTagRemoval+=1;
        stats_.numTooShortReads+=1;
        return false;
    }
    BamTools::BamAlignment bamAl;


    boost::xpressive::smatch what;
    std::string query = entry.sequence.substr(barcodeLength_);

    //does regex hit query?
    if(boost::xpressive::regex_search(query.begin(), query.end(), what, rex_))
    {
        //only remove if a sensible length (i.e. <30), otherwise it could span 100s of bases (which is unlikely to be tag!)
        if(what.length(0) < 30)
        {
            stats_.numReadsWithTags+=1;
            query = query.substr(what.length(0));
            //final check after tag removal to see if sequence too short
            if(query.size() < minReadLength_)
            {
                stats_.numTooShortReadsAfterTagRemoval+=1;
                stats_.numTooShortReads+=1;
                return false;
            }
        }
    }
    //if not
    else
    {
//        query = entry.sequence;
        stats_.numReadsWithoutTags+=1;
        stats_.numOkSizeReads+=1;
    }



    bamAl.Qualities = entry.quality.substr(barcodeLength_);
    bamAl.QueryBases = entry.sequence.substr(barcodeLength_);
    bamAl.Name = entry.header1;
    bamAl.SetIsMapped(false);
    std::string barcode = entry.sequence.substr(0,barcodeLength_);
    bamAl.AddTag<std::string>("BC","Z",barcode);
    bamAl.AddTag<std::string>("QT","Z",entry.quality.substr(0,barcodeLength_));
    bamAl.Length = entry.sequence.size();

    //export
    std::map<std::string, BamTools::BamWriter*>::iterator i = outputParser_.barcode_file_map.find(barcode);
    if(i==outputParser_.barcode_file_map.end())
    {
        //check shortestRead has been initialised
        if(stats_.shortestReadLenForBarcode["NM"]==0) stats_.shortestReadLenForBarcode["NM"]=16000;

        //save info
        stats_.numReadsForBarcode["NM"]+=1;
        stats_.totalBasesForBarcode["NM"]+=query.size();
        if(query.size() < stats_.shortestReadLenForBarcode["NM"]) stats_.shortestReadLenForBarcode["NM"] = query.size();
        else if(query.size() > stats_.longestReadLenForBarcode["NM"]) stats_.longestReadLenForBarcode["NM"] = query.size();

        //output read
        outputParser_.barcode_file_map["NM"]->SaveAlignment(bamAl);
    }
    else
    {
        //check shortestRead has been initialised
        if(stats_.shortestReadLenForBarcode[i->first]==0) stats_.shortestReadLenForBarcode[i->first]=16000;
        stats_.numReadsForBarcode[i->first]+=1;
        stats_.totalBasesForBarcode[i->first]+=query.size();
        if(query.size() < stats_.shortestReadLenForBarcode[i->first]) stats_.shortestReadLenForBarcode[i->first] = query.size();
        else if(query.size() > stats_.longestReadLenForBarcode[i->first]) stats_.longestReadLenForBarcode[i->first] = query.size();

        //output read
        i->second->SaveAlignment(bamAl);
    }

    //check read lengths
    int readLen = query.size();

    if(readLen < stats_.shortestReadLen) stats_.shortestReadLen = readLen;
    else if(readLen > stats_.longestReadLen) stats_.longestReadLen = readLen;


    stats_.totalBases+=readLen;
    return true;

}



//! Closes all export file handles

//! Deletes each file handle so no memory leakage.
void BamExporter::closeFiles()
{
    std::map<std::string, BamTools::BamWriter*>::iterator iO = outputParser_.barcode_file_map.begin();
    while(iO!=outputParser_.barcode_file_map.end())
    {
        iO->second->Close();
        delete iO->second;
        ++iO;
    }
}



//! Getter for internal stats bundle class

//!@return Reference to stats bundle
TorrentReadStatsBundle& BamExporter::getStats()
{
    return stats_;
}



//! Getter for internal output parser class

//!@return Reference to output parser
OutputParser& BamExporter::getOutputParser()
{
    return outputParser_;
}
