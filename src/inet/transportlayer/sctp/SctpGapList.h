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

    void check(const uint32 cTsnAck) const;
    void print(std::ostream& os) const;

    uint32 getNumGaps() const
    {
        return NumGaps;
    }

    uint32 getGapStart(const uint32 index) const
    {
        assert(index < NumGaps);
        return GapStartList[index];
    }

    uint32 getGapStop(const uint32 index) const
    {
        assert(index < NumGaps);
        return GapStopList[index];
    }

    bool tsnInGapList(const uint32 tsn) const;
    void forwardCumAckTsn(const uint32 cTsnAck);
    bool tryToAdvanceCumAckTsn(uint32& cTsnAck);
    void removeFromGapList(const uint32 removedTsn);
    bool updateGapList(const uint32 receivedTsn,
            uint32& cTsnAck,
            bool& newChunkReceived);
    void resetGaps();

    // ====== Private data ===================================================

  private:
    uint32 NumGaps;
    uint32 GapStartList[MAX_GAP_COUNT];
    uint32 GapStopList[MAX_GAP_COUNT];
};

inline std::ostream& operator<<(std::ostream& ostr, const SctpSimpleGapList& gapList) { gapList.print(ostr); return ostr; }

class INET_API SctpGapList
{
  public:
    SctpGapList();
    ~SctpGapList();

    void setInitialCumAckTsn(const uint32 cumAckTsn)
    {
        assert(CombinedGapList.getNumGaps() == 0);
        CumAckTsn = cumAckTsn;
    }

    uint32 getCumAckTsn() const
    {
        return CumAckTsn;
    }

    uint32 getHighestTsnReceived() const
    {
        if (CombinedGapList.getNumGaps() > 0) {
            return CombinedGapList.getGapStop(CombinedGapList.getNumGaps() - 1);
        }
        else {
            return CumAckTsn;
        }
    }

    enum GapType {
        GT_Any = 0,
        GT_Revokable = 1,
        GT_NonRevokable = 2
    };

    uint32 getNumGaps(const GapType type) const
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

    bool tsnInGapList(const uint32 tsn) const
    {
        return CombinedGapList.tsnInGapList(tsn);
    }

    bool tsnIsRevokable(const uint32 tsn) const
    {
        return RevokableGapList.tsnInGapList(tsn);
    }

    bool tsnIsNonRevokable(const uint32 tsn) const
    {
        return NonRevokableGapList.tsnInGapList(tsn);
    }

    uint32 getGapStart(const GapType type, const uint32 index) const
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

    uint32 getGapStop(const GapType type, const uint32 index) const
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

    void forwardCumAckTsn(const uint32 cumAckTsn);
    bool tryToAdvanceCumAckTsn();
    void removeFromGapList(const uint32 removedTsn);
    bool updateGapList(const uint32 receivedTsn,
            bool& newChunkReceived,
            bool tsnIsRevokable = true);
    void resetGaps(const uint32 newCumAck);

    // ====== Private data ===================================================

  private:
    uint32 CumAckTsn;
    SctpSimpleGapList RevokableGapList;
    SctpSimpleGapList NonRevokableGapList;
    SctpSimpleGapList CombinedGapList;
};

inline std::ostream& operator<<(std::ostream& ostr, const SctpGapList& gapList) { gapList.print(ostr); return ostr; }

} // namespace sctp
} // namespace inet

#endif // ifndef __INET_SCTPGAPLIST_H

