#include "TorrentReadStatsBundle.hpp"
#include <iostream>

//! Constructor

//! Initialises all variables that require it.
TorrentReadStatsBundle::TorrentReadStatsBundle()
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
//        //need to initialise shortestRead for each barcode as a large number
//        std::map<std::string, int>::iterator i = barcodeMap.begin();
//        while(i!=barcodeMap.end())
//        {
//            numReadsForBarcode.insert(std::pair<std::string,long>(i->first,0));
//            ++i;
//        }
//        numReadsForBarcode.insert(std::pair<std::string,long>("NM",0));
//    }

//! Writes basic stats to console

//! Reports # reads, # good reads, # bad reads (too short), # reads too short before tag/barcode removal, # reads too short after tag/barcode removal, # reads with tags, # reads without tags.

//! For debugging only. Deprecated.
void TorrentReadStatsBundle::report()
{
    std::cout << numReads << "\t" << numOkSizeReads << "\t" << numTooShortReads << "\t" << numTooShortReadsBeforeTagRemoval << "\t" << numTooShortReadsAfterTagRemoval << "\t" << numReadsWithTags << "\t" << numReadsWithoutTags;
}



//! Writes barcode stats to console

//! Reports # good reads for each barcode.

//! For debugging only. Deprecated.
void TorrentReadStatsBundle::reportBarcodeFrequencies()
{
    std::map<std::string, long>::iterator i = numReadsForBarcode.begin();
    while(i!=numReadsForBarcode.end())
    {
        std::cout << " [" << i->first << ":" << i->second << "] ";
        ++i;
    }
    std::cout << std::endl;
}



//! Performs final stats calculations on bundle

//! Calculates average read length sizes for run and for each barcode.
void TorrentReadStatsBundle::generateFinalStats()
{
    averageReadLen = totalBases / numOkSizeReads;
    std::map<std::string, int>::iterator iTotal=totalBasesForBarcode.begin();
    while(iTotal!=totalBasesForBarcode.end())
    {
        averageReadLenForBarcode[iTotal->first]=iTotal->second / numReadsForBarcode[iTotal->first];
        ++iTotal;
    }
}



//! Writes read length stats to console

//! For debugging only. Deprecated
void TorrentReadStatsBundle::reportReadLengthStats()
{
    std::cout << "Shortest: " << shortestReadLen << "\tLongest: " << longestReadLen << "\tAverage: " << averageReadLen << "\tTotal: " << totalBases;
}
