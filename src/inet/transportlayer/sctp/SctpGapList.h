//
// Copyright (C) 2010-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCTPGAPLIST_H
#define __INET_SCTPGAPLIST_H

#include <assert.h>

#include "inet/common/INETDefs.h"

namespace inet {
namespace sctp {

//#include "SctpSeqNumbers.h"

#define MAX_GAP_COUNT    500

class INET_API SctpSimpleGapList
{
  public:
    SctpSimpleGapList();
    ~SctpSimpleGapList();

    void check(const uint32_t cTsnAck) const;
    void print(std::ostream& os) const;

    uint32_t getNumGaps() const
    {
        return NumGaps;
    }

    uint32_t getGapStart(const uint32_t index) const
    {
        assert(index < NumGaps);
        return GapStartList[index];
    }

    uint32_t getGapStop(const uint32_t index) const
    {
        assert(index < NumGaps);
        return GapStopList[index];
    }

    bool tsnInGapList(const uint32_t tsn) const;
    void forwardCumAckTsn(const uint32_t cTsnAck);
    bool tryToAdvanceCumAckTsn(uint32_t& cTsnAck);
    void removeFromGapList(const uint32_t removedTsn);
    bool updateGapList(const uint32_t receivedTsn,
            uint32_t& cTsnAck,
            bool& newChunkReceived);
    void resetGaps();

    // ====== Private data ===================================================

  private:
    uint32_t NumGaps;
    uint32_t GapStartList[MAX_GAP_COUNT];
    uint32_t GapStopList[MAX_GAP_COUNT];
};

inline std::ostream& operator<<(std::ostream& ostr, const SctpSimpleGapList& gapList) { gapList.print(ostr); return ostr; }

class INET_API SctpGapList
{
  public:
    SctpGapList();
    ~SctpGapList();

    void setInitialCumAckTsn(const uint32_t cumAckTsn)
    {
        assert(CombinedGapList.getNumGaps() == 0);
        CumAckTsn = cumAckTsn;
    }

    uint32_t getCumAckTsn() const
    {
        return CumAckTsn;
    }

    uint32_t getHighestTsnReceived() const
    {
        if (CombinedGapList.getNumGaps() > 0) {
            return CombinedGapList.getGapStop(CombinedGapList.getNumGaps() - 1);
        }
        else {
            return CumAckTsn;
        }
    }

    enum GapType {
        GT_Any          = 0,
        GT_Revokable    = 1,
        GT_NonRevokable = 2
    };

    uint32_t getNumGaps(const GapType type) const
    {
        if (type == GT_Revokable) {
            return RevokableGapList.getNumGaps();
        }
        else if (type == GT_NonRevokable) {
            return NonRevokableGapList.getNumGaps();
        }
        else {
            return CombinedGapList.getNumGaps();
        }
    }

    bool tsnInGapList(const uint32_t tsn) const
    {
        return CombinedGapList.tsnInGapList(tsn);
    }

    bool tsnIsRevokable(const uint32_t tsn) const
    {
        return RevokableGapList.tsnInGapList(tsn);
    }

    bool tsnIsNonRevokable(const uint32_t tsn) const
    {
        return NonRevokableGapList.tsnInGapList(tsn);
    }

    uint32_t getGapStart(const GapType type, const uint32_t index) const
    {
        if (type == GT_Revokable) {
            return RevokableGapList.getGapStart(index);
        }
        else if (type == GT_NonRevokable) {
            return NonRevokableGapList.getGapStart(index);
        }
        else {
            return CombinedGapList.getGapStart(index);
        }
    }

    uint32_t getGapStop(const GapType type, const uint32_t index) const
    {
        if (type == GT_Revokable) {
            return RevokableGapList.getGapStop(index);
        }
        else if (type == GT_NonRevokable) {
            return NonRevokableGapList.getGapStop(index);
        }
        else {
            return CombinedGapList.getGapStop(index);
        }
    }

    void check() const;
    void print(std::ostream& os) const;

    void forwardCumAckTsn(const uint32_t cumAckTsn);
    bool tryToAdvanceCumAckTsn();
    void removeFromGapList(const uint32_t removedTsn);
    bool updateGapList(const uint32_t receivedTsn,
            bool& newChunkReceived,
            bool tsnIsRevokable = true);
    void resetGaps(const uint32_t newCumAck);

    // ====== Private data ===================================================

  private:
    uint32_t CumAckTsn;
    SctpSimpleGapList RevokableGapList;
    SctpSimpleGapList NonRevokableGapList;
    SctpSimpleGapList CombinedGapList;
};

inline std::ostream& operator<<(std::ostream& ostr, const SctpGapList& gapList) { gapList.print(ostr); return ostr; }

} // namespace sctp
} // namespace inet

#endif

