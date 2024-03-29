//
// Copyright (C) 2010-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/sctp/SctpGapList.h"

#include "inet/transportlayer/sctp/SctpAssociation.h"

namespace inet {
namespace sctp {

// ###### Constructor #######################################################
SctpSimpleGapList::SctpSimpleGapList()
{
    NumGaps = 0;
    for (unsigned int i = 0; i < MAX_GAP_COUNT; i++) {
        GapStartList[i] = 0xffff00ff;
        GapStopList[i] = 0xffff0000;
    }
}

// ###### Destructor ########################################################
SctpSimpleGapList::~SctpSimpleGapList()
{
}

// ###### Check gap list ####################################################
void SctpSimpleGapList::check(const uint32_t cTsnAck) const
{
    for (uint32_t i = 0; i < NumGaps; i++) {
        if (i == 0) {
            ASSERT(SctpAssociation::tsnGt(GapStartList[i], cTsnAck + 1));
        }
        else {
            ASSERT(SctpAssociation::tsnGt(GapStartList[i], GapStopList[i - 1] + 1));
        }
        ASSERT(SctpAssociation::tsnLe(GapStartList[i], GapStopList[i]));
    }
}

void SctpSimpleGapList::resetGaps()
{
    NumGaps = 0;
    for (unsigned int i = 0; i < MAX_GAP_COUNT; i++) {
        GapStartList[i] = 0xffff00ff;
        GapStopList[i] = 0xffff0000;
    }
}

// ###### Print gap list ####################################################
void SctpSimpleGapList::print(std::ostream& os) const
{
    os << "{";
    for (uint32_t i = 0; i < NumGaps; i++) {
        if (i > 0) {
            os << ",";
        }
        os << " " << GapStartList[i] << "-" << GapStopList[i];
    }
    os << " }";
}

// ###### Is TSN in gap list? ###############################################
bool SctpSimpleGapList::tsnInGapList(const uint32_t tsn) const
{
    for (uint32_t i = 0; i < NumGaps; i++) {
        if (SctpAssociation::tsnBetween(GapStartList[i], tsn, GapStopList[i])) {
            return true;
        }
    }
    return false;
}

// ###### Forward CumAckTSN #################################################
void SctpSimpleGapList::forwardCumAckTsn(const uint32_t cTsnAck)
{
    if (NumGaps > 0) {
        // It is only possible to advance CumAckTsn when there are gaps.
        uint32_t counter = 0;
        uint32_t advance = 0;
        while (counter < NumGaps) {
            // Check whether CumAckTsn can be advanced.
            if (SctpAssociation::tsnGe(cTsnAck, GapStartList[counter])) { // Yes!
                advance++;
            }
            else { // No -> end of search.
                break;
            }
            counter++;
        }

        if (advance > 0) {
            // We can remove "advance" block now.
            for (uint32_t i = advance; i < NumGaps; i++) {
                GapStartList[i - advance] = GapStartList[i];
                GapStopList[i - advance] = GapStopList[i];
            }
            NumGaps -= advance;
        }
    }
}

// ###### Try to advance CumAckTsn ##########################################
bool SctpSimpleGapList::tryToAdvanceCumAckTsn(uint32_t& cTsnAck)
{
    bool progress = false;
    if (NumGaps > 0) {
        // It is only possible to advance CumAckTsn when there are gaps.
        uint32_t counter = 0;
        while (counter < NumGaps) {
            // Check whether CumAckTsn can be advanced.
            if (cTsnAck + 1 == GapStartList[0]) { // Yes!
                cTsnAck = GapStopList[0];
                // We can take out all fragments of this block
                for (uint32_t i = 1; i < NumGaps; i++) {
                    GapStartList[i - 1] = GapStartList[i];
                    GapStopList[i - 1] = GapStopList[i];
                }
                progress = true;
            }
            counter++;
        }
    }
    return progress;
}

// ###### Remove Tsn from gap list ##########################################
void SctpSimpleGapList::removeFromGapList(const uint32_t removedTsn)
{
    const int32_t initialNumGaps = NumGaps;

    for (int32_t i = initialNumGaps - 1; i >= 0; i--) {
        if (SctpAssociation::tsnBetween(GapStartList[i], removedTsn, GapStopList[i])) {
            // ====== Gap block contains more than one TSN =====================
            const int32_t gapsize = (int32_t)(GapStopList[i] - GapStartList[i] + 1);
            if (gapsize > 1) {
                if (GapStopList[i] == removedTsn) { // Remove stop TSN
                    GapStopList[i]--;
                }
                else if (GapStartList[i] == removedTsn) { // Remove start TSN
                    GapStartList[i]++;
                }
                else { // Block has to be splitted up
                    NumGaps = std::min(NumGaps + 1, (uint32_t)MAX_GAP_COUNT); // Enforce upper limit!
                    for (int32_t j = NumGaps - 1; j > i; j--) {
                        GapStopList[j] = GapStopList[j - 1];
                        GapStartList[j] = GapStartList[j - 1];
                    }
                    GapStopList[i] = removedTsn - 1;
                    if ((uint32_t)i + 1 < NumGaps) {
                        GapStartList[i + 1] = removedTsn + 1;
                    }
                }
            }
            // ====== Just a single TSN in the gap block (start==stop) =========
            else {
                for (int32_t j = i; j <= initialNumGaps - 1; j++) {
                    GapStopList[j] = GapStopList[j + 1];
                    GapStartList[j] = GapStartList[j + 1];
                }
                GapStartList[initialNumGaps - 1] = 0;
                GapStopList[initialNumGaps - 1] = 0;
                NumGaps--;
            }

            break; // TSN removed -> done!
        }
    }
}

// ###### Add TSN to gap list ###############################################
bool SctpSimpleGapList::updateGapList(const uint32_t receivedTsn,
        uint32_t& cTsnAck,
        bool& newChunkReceived)
{
    if (SctpAssociation::tsnLe(receivedTsn, cTsnAck)) {
        // Received TSN covered by CumAckTSN -> nothing to do.
        return false;
    }

    uint32_t lo = cTsnAck + 1;
    for (uint32_t i = 0; i < NumGaps; i++) {
        if (GapStartList[i] > 0) {
            const uint32_t hi = GapStartList[i] - 1;
            if (SctpAssociation::tsnBetween(lo, receivedTsn, hi)) {
                const uint32_t gapsize = hi - lo + 1;
                if (gapsize > 1) {
                    /**
                     * TSN either sits at the end of one gap, and thus changes gap
                     * boundaries, or it is in between two gaps, and becomes a new gap
                     */
                    if (receivedTsn == hi) {
                        GapStartList[i] = receivedTsn;
                        newChunkReceived = true;
                        return true;
                    }
                    else if (receivedTsn == lo) {
                        if (receivedTsn == (cTsnAck + 1)) {
                            cTsnAck++;
                            newChunkReceived = true;
                            return true;
                        }
                        /* some gap must increase its upper bound */
                        GapStopList[i - 1] = receivedTsn;
                        newChunkReceived = true;
                        return true;
                    }
                    else { /* a gap in between */
                        NumGaps = std::min(NumGaps + 1, (uint32_t)MAX_GAP_COUNT); // Enforce upper limit!

                        for (uint32_t j = NumGaps - 1; j > i; j--) {
                            GapStartList[j] = GapStartList[j - 1];
                            GapStopList[j] = GapStopList[j - 1];
                        }
                        GapStartList[i] = receivedTsn;
                        GapStopList[i] = receivedTsn;
                        newChunkReceived = true;
                        return true;
                    }
                }
                else { /* alright: gapsize is 1: our received tsn may close gap between fragments */
                    if (lo == cTsnAck + 1) {
                        cTsnAck = GapStopList[i];
                        if (i == NumGaps - 1) {
                            GapStartList[i] = 0;
                            GapStopList[i] = 0;
                        }
                        else {
                            for (uint32_t j = i; j < NumGaps - 1; j++) {
                                GapStartList[j] = GapStartList[j + 1];
                                GapStopList[j] = GapStopList[j + 1];
                            }
                        }
                        NumGaps--;
                        newChunkReceived = true;
                        return true;
                    }
                    else {
                        GapStopList[i - 1] = GapStopList[i];
                        if (i == NumGaps - 1) {
                            GapStartList[i] = 0;
                            GapStopList[i] = 0;
                        }
                        else {
                            for (uint32_t j = i; j < NumGaps - 1; j++) {
                                GapStartList[j] = GapStartList[j + 1];
                                GapStopList[j] = GapStopList[j + 1];
                            }
                        }
                        NumGaps--;
                        newChunkReceived = true;
                        return true;
                    }
                }
            }
            else { /* receivedTsn is not in the gap between these fragments... */
                lo = GapStopList[i] + 1;
            }
        } /* end: for */
    } /* end: for */

    // ====== We have reached the end of the list ============================
    if (receivedTsn == lo) { // just increase CumAckTsn, handle further update of CumAckTsn later
        if (receivedTsn == cTsnAck + 1) {
            cTsnAck = receivedTsn;
            newChunkReceived = true;
            return true;
        }
        // Update last fragment, stop TSN by one.
        GapStopList[NumGaps - 1]++;

        newChunkReceived = true;
        return true;
    }
    else {
        // If the received TSN is new, the list is either empty or the received
        // TSN is larger than the last gap end + 1. Otherwise, the TSN has already
        // been acknowledged.
        if ((NumGaps == 0) ||
            (SctpAssociation::tsnGt(receivedTsn, GapStopList[NumGaps - 1] + 1)))
        {
            // A new fragment altogether, past the end of the list
            if (NumGaps < MAX_GAP_COUNT) { // T.D. 18.12.09: Enforce upper limit!
                GapStartList[NumGaps] = receivedTsn;
                GapStopList[NumGaps] = receivedTsn;
                NumGaps++;
                newChunkReceived = true;
            }
        }
        return true;
    }

    return false;
}

// ###### Constructor #######################################################
SctpGapList::SctpGapList()
{
    CumAckTsn = 0;
}

// ###### DestruSctpGapListctor ########################################################
SctpGapList::~SctpGapList()
{
}

// ###### Check gap list ####################################################
void SctpGapList::check() const
{
    CombinedGapList.check(CumAckTsn);
    RevokableGapList.check(CumAckTsn);
    NonRevokableGapList.check(CumAckTsn);
}

// ###### Print gap list ####################################################
void SctpGapList::print(std::ostream& os) const
{
    os << "CumAck=" << CumAckTsn;
    os << "   Combined-Gaps=" << CombinedGapList;
    os << "   R-Gaps=" << RevokableGapList;
    os << "   NR-Gaps=" << NonRevokableGapList;
}

// ###### Forward CumAckTsn #################################################
void SctpGapList::forwardCumAckTsn(const uint32_t cumAckTsn)
{
    CumAckTsn = cumAckTsn;
    CombinedGapList.forwardCumAckTsn(CumAckTsn);
    RevokableGapList.forwardCumAckTsn(CumAckTsn);
    NonRevokableGapList.forwardCumAckTsn(CumAckTsn);
}

// ###### Advance CumAckTsn #################################################
bool SctpGapList::tryToAdvanceCumAckTsn()
{
    if (CombinedGapList.tryToAdvanceCumAckTsn(CumAckTsn)) {
        RevokableGapList.forwardCumAckTsn(CumAckTsn);
        NonRevokableGapList.forwardCumAckTsn(CumAckTsn);
        return true;
    }
    return false;
}

// ###### Remove TSN from gap list ##########################################
void SctpGapList::removeFromGapList(const uint32_t removedTsn)
{
    RevokableGapList.removeFromGapList(removedTsn);
    NonRevokableGapList.removeFromGapList(removedTsn);
    CombinedGapList.removeFromGapList(removedTsn);
}

// ###### Add TSN to gap list ###############################################
bool SctpGapList::updateGapList(const uint32_t receivedTsn,
        bool& newChunkReceived,
        bool tsnIsRevokable)
{
    uint32_t oldCumAckTsn = CumAckTsn;
    if (tsnIsRevokable) {
        // Once a TSN become non-revokable, it cannot become revokable again!
        // However, if the list became too long, updateGapList() may be called
        // again when the chunk is received again.
        RevokableGapList.updateGapList(receivedTsn, oldCumAckTsn, newChunkReceived);
    }
    else {
        if (NonRevokableGapList.updateGapList(receivedTsn, oldCumAckTsn, newChunkReceived) == true) {
            // TSN has moved from revokable to non-revokable!
            RevokableGapList.removeFromGapList(receivedTsn);
        }
    }

    // Finally, add TSN to combined list and set CumAckTSN.
    oldCumAckTsn = CumAckTsn;
    const bool newChunk = CombinedGapList.updateGapList(receivedTsn, CumAckTsn, newChunkReceived);
    if (oldCumAckTsn != CumAckTsn) {
        RevokableGapList.forwardCumAckTsn(CumAckTsn);
        NonRevokableGapList.forwardCumAckTsn(CumAckTsn);
    }
    return newChunk;
}

void SctpGapList::resetGaps(const uint32_t newCumAck)
{
    CumAckTsn = newCumAck;
    RevokableGapList.resetGaps();
    NonRevokableGapList.resetGaps();
    CombinedGapList.resetGaps();
}

} // namespace sctp
} // namespace inet

