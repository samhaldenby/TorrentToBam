#ifndef TORRENTREADSTATSBUNDLE_HPP_INCLUDED
#define TORRENTREADSTATSBUNDLE_HPP_INCLUDED

struct TorrentReadStatsBundle
{
    //filtering variables
    long numReads;
    long numTooShortReads;
    long numTooShortReadsBeforeTagRemoval;
    long numTooShortReadsAfterTagRemoval;
    long numOkSizeReads;
    long numReadsWithTags;
    long numReadsWithoutTags;

    //barcode variables
    std::map<std::string, long> numReadsForBarcode;

    //read length variables
    int shortestReadLen;
    int longestReadLen;
    int averageReadLen;
    int totalBases;

    TorrentReadStatsBundle()
    {
        numReads = 0;
        numTooShortReads = 0;
        numTooShortReadsBeforeTagRemoval = 0;
        numTooShortReadsAfterTagRemoval = 0;
        numOkSizeReads = 0;
        numReadsWithTags = 0;
        numReadsWithoutTags = 0;
        shortestReadLen = 16000;
        longestReadLen = 0;
        averageReadLen = 0;
        totalBases = 0;
    }

//    void initBarcodes(std::map<std::string, std::string> barcodeMap)
//    {
//        std::map<std::string, std::string>::iterator i = barcodeMap.begin();
//        while(i!=barcodeMap.end())
//        {
//            numReadsForBarcode.insert(std::pair<std::string,long>(i->first,0));
//            ++i;
//        }
//        numReadsForBarcode.insert(std::pair<std::string,long>("NM",0));
//    }

    void report()
    {
        std::cout << numReads << "\t" << numOkSizeReads << "\t" << numTooShortReads << "\t" << numTooShortReadsBeforeTagRemoval << "\t" << numTooShortReadsAfterTagRemoval << "\t" << numReadsWithTags << "\t" << numReadsWithoutTags;
    }

    void reportBarcodeFrequencies()
    {
        std::map<std::string, long>::iterator i = numReadsForBarcode.begin();
        while(i!=numReadsForBarcode.end())
        {
            std::cout << " [" << i->first << ":" << i->second << "] ";
            ++i;
        }
        std::cout << std::endl;
    }

    void generateFinalStats()
    {
        averageReadLen = totalBases / numOkSizeReads;
    }

    void reportReadLengthStats()
    {
        std::cout << "Shortest: " << shortestReadLen << "\tLongest: " << longestReadLen << "\tAverage: " << averageReadLen << "\tTotal: " << totalBases;
    }
};

#endif // TORRENTREADSTATSBUNDLE_HPP_INCLUDED
