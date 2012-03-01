#include "BamExporter.hpp"

BamExporter::BamExporter(int barcodeLength, int minReadLength) : barcodeLength_(barcodeLength), minReadLength_(minReadLength)
{
    //set barcode regex
    rex_ = boost::xpressive::bol >> "TGTA" >> boost::xpressive::_w >> *(~boost::xpressive::_n) >> "CAGT";
}



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
            std::string fileName = name + "_" + barcode + ".bam";
            pO->Open(fileName, header_, refs_);
            outputParser_.barcode_file_map[barcode]=pO;

        }
        barcode="";
        name="";
    }

    //add non-match stuff
    std::string fileName = "NonMatched.bam";
    BamTools::BamWriter* pO = new BamTools::BamWriter();
    pO->Open(fileName, header_, refs_);
    outputParser_.barcode_file_map["NM"]=pO;

    return true;

}



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

//    sregex rex = bol >> "TGTA" >> _w >> *(~_n) >> "CAGT";
    boost::xpressive::smatch what;
    std::string query = entry.sequence.substr(barcodeLength_);

    //does regex hit query?
    if(boost::xpressive::regex_search(query.begin(), query.end(), what, rex_))
    {
//        std::cout << "size: " << what.size() << "\t" << what.length(0) << "\t" << query.size() << std::endl;
//        std::cout << "TRUE: (" << what[1] << ")\t" << what[0] << std::endl;
//        std::cout << what.length(0) << std::endl;
        //only remove if a sensible length (i.e. <30), otherwise it could span 100s of bases (which is unlikely to be tag!)
        if(what.length(0) < 30)
        {
//            std::cout << query << "\t" << query.substr(what.length(0)) << std::endl;
            query = query.substr(what.length(0));
            //final check after tag removal to see if sequence too short
            if(query.size() < minReadLength_)
            {
                stats_.numTooShortReadsAfterTagRemoval+=1;
                stats_.numTooShortReads+=1;
                return false;
            }
        }

//        regexMatch+=1;
    }
    //if not
    else
    {
        query = entry.sequence;
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
        outputParser_.barcode_file_map["NM"]->SaveAlignment(bamAl);
    }
    else
    {
        i->second->SaveAlignment(bamAl);
    }

    return true;

}


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

TorrentReadStatsBundle& BamExporter::getStats()
{
    return stats_;
}
