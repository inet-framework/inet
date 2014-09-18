//
// Copyright (C) 2010-2012 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __INET_SCTPGAPLIST_H
#define __INET_SCTPGAPLIST_H

#include <omnetpp.h>
#include <assert.h>

namespace inet {

namespace sctp {

//#include "SCTPSeqNumbers.h"

#define MAX_GAP_COUNT    500

class SCTPSimpleGapList
{
  public:
    SCTPSimpleGapList();
    ~SCTPSimpleGapList();

    void check(const uint32 cTsnAck) const;
    void print(std::ostream& os) const;

    inline uint32 getNumGaps() const
    {
        return NumGaps;
    }

    inline uint32 getGapStart(const uint32 index) const
    {
        assert(index < NumGaps);
        return GapStartList[index];
    }

    inline uint32 getGapStop(const uint32 index) const
    {
        assert(index < NumGaps);
        return GapStopList[index];
    }

    bool tsnInGapList(const uint32 tsn) const;
    void forwardCumAckTSN(const uint32 cTsnAck);
    bool tryToAdvanceCumAckTSN(uint32& cTsnAck);
    void removeFromGapList(const uint32 removedTSN);
    bool updateGapList(const uint32 receivedTSN,
            uint32& cTsnAck,
            bool& newChunkReceived);

    // ====== Private data ===================================================

  private:
    uint32 NumGaps;
    uint32 GapStartList[MAX_GAP_COUNT];
    uint32 GapStopList[MAX_GAP_COUNT];
};

inline std::ostream& operator<<(std::ostream& ostr, const SCTPSimpleGapList& gapList) { gapList.print(ostr); return ostr; }

class SCTPGapList
{
  public:
    SCTPGapList();
    ~SCTPGapList();

    inline void setInitialCumAckTSN(const uint32 cumAckTSN)
    {
        assert(CombinedGapList.getNumGaps() == 0);
        CumAckTSN = cumAckTSN;
    }

    inline uint32 getCumAckTSN() const
    {
        return CumAckTSN;
    }

    inline uint32 getHighestTSNReceived() const
    {
        if (CombinedGapList.getNumGaps() > 0) {
            return CombinedGapList.getGapStop(CombinedGapList.getNumGaps() - 1);
        }
        else {
            return CumAckTSN;
        }
    }

    enum GapType {
        GT_Any = 0,
        GT_Revokable = 1,
        GT_NonRevokable = 2
    };

    inline uint32 getNumGaps(const GapType type) const
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

    inline bool tsnInGapList(const uint32 tsn) const
    {
        return CombinedGapList.tsnInGapList(tsn);
    }

    inline bool tsnIsRevokable(const uint32 tsn) const
    {
        return RevokableGapList.tsnInGapList(tsn);
    }

    inline bool tsnIsNonRevokable(const uint32 tsn) const
    {
        return NonRevokableGapList.tsnInGapList(tsn);
    }

    inline uint32 getGapStart(const GapType type, const uint32 index) const
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

    inline uint32 getGapStop(const GapType type, const uint32 index) const
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

    void forwardCumAckTSN(const uint32 cumAckTSN);
    bool tryToAdvanceCumAckTSN();
    void removeFromGapList(const uint32 removedTSN);
    bool updateGapList(const uint32 receivedTSN,
            bool& newChunkReceived,
            bool tsnIsRevokable = true);

    // ====== Private data ===================================================

  private:
    uint32 CumAckTSN;
    SCTPSimpleGapList RevokableGapList;
    SCTPSimpleGapList NonRevokableGapList;
    SCTPSimpleGapList CombinedGapList;
};

inline std::ostream& operator<<(std::ostream& ostr, const SCTPGapList& gapList) { gapList.print(ostr); return ostr; }

} // namespace sctp

} // namespace inet

#endif // ifndef __INET_SCTPGAPLIST_H

