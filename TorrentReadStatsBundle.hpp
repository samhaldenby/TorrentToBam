#ifndef TORRENTREADSTATSBUNDLE_HPP_INCLUDED
#define TORRENTREADSTATSBUNDLE_HPP_INCLUDED

struct TorrentReadStatsBundle
{
    long numReads;
    long numTooShortReads;
    long numTooShortReadsBeforeTagRemoval;
    long numTooShortReadsAfterTagRemoval;
    long numOkSizeReads;

    TorrentReadStatsBundle()
    {
        numReads = 0;
        numTooShortReads = 0;
        numTooShortReadsBeforeTagRemoval = 0;
        numTooShortReadsAfterTagRemoval = 0;
        numOkSizeReads = 0;
    }

    void report()
    {
        std::cout << numReads << "\t" << numOkSizeReads << "\t" << numTooShortReads << "\t" << numTooShortReadsBeforeTagRemoval << "\t" << numTooShortReadsAfterTagRemoval;
    }
};

#endif // TORRENTREADSTATSBUNDLE_HPP_INCLUDED
