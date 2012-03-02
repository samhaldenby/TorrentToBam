#ifndef TORRENTREADSTATSBUNDLE_HPP_INCLUDED
#define TORRENTREADSTATSBUNDLE_HPP_INCLUDED

#include <map>
#include <string>

struct TorrentReadStatsBundle
{
    //filtering variables
    long numReads;     //!< Total number of reads including those too short.
    long numTooShortReads;  //!< Total number of reads that are shorter than minimum required read length either before or after tag/barcode trimming.
    long numTooShortReadsBeforeTagRemoval;  //!< Total number of reads that are shorter than minimum required read length prior to tag/barcode trimming.
    long numTooShortReadsAfterTagRemoval;   //!< Total number of reads that are shorter than minimum required read length after tag/barcode trimming.
    long numOkSizeReads;    //!< Total number of reads that met required read length critera
    long numReadsWithTags;  //!< Total number of reads that contained a tag sequence
    long numReadsWithoutTags;   //!< Total number of reads that did not contain a tag sequence

    //read length variables
    int shortestReadLen;    //!< Length of shortest exported read from run
    int longestReadLen; //!< Length of longest exported read from run
    int averageReadLen; //!< Average length of exported reads from run
    int totalBases; //!< Total number of exported bases from run
    //barcode variables
    std::map<std::string, long> numReadsForBarcode; //!< Number of reads exported for each barcoded sample
    std::map<std::string, int> shortestReadLenForBarcode; //!< Length of shortest read exported for each barcoded sample
    std::map<std::string, int> longestReadLenForBarcode; //!< Length of longest read exported for each barcoded sample
    std::map<std::string, int> averageReadLenForBarcode;    //!< Average length of exported reads for each barcoded sample
    std::map<std::string, int> totalBasesForBarcode;    //!< Number of exported bases for each sample

    //functions
    TorrentReadStatsBundle();
    void report();
    void reportBarcodeFrequencies();
    void reportReadLengthStats();
    void generateFinalStats();

};


//    void initBarcodes(std::map<std::string, std::string> barcodeMap)
//    {
//        //need to initialise shortestRead for each barcode as a large number
//        std::map<std::string, int>::iterator i = barcodeMap.begin();
//        while(i!=barcodeMap.end())
//        {
//            numReadsForBarcode.insert(std::pair<std::string,long>(i->first,0));
//            ++i;
//        }
//        numReadsForBarcode.insert(std::pair<std::string,long>("NM",0));
//    }


#endif // TORRENTREADSTATSBUNDLE_HPP_INCLUDED
