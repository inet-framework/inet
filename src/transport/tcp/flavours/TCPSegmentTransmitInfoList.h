//
// Copyright (C) 2013 OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// @author Zoltan Bojthe
//

#ifndef __INET_TCPSEGMENTTRANSMITINFOLIST_H
#define __INET_TCPSEGMENTTRANSMITINFOLIST_H


#include "INETDefs.h"

#include "TCPBaseAlg.h"

class TCPSegmentTransmitInfoList
{
  protected:
    class TCPSegmentTransmitInfo
    {
      public:
        uint32_t beg;       // segment [begin, end)
        uint32_t end;
        simtime_t senttime; // time of first sending
        int transmits;      // num of transmissions
      public:
        TCPSegmentTransmitInfo(uint32_t beg, uint32_t end, simtime_t senttime, int transmits) : beg(beg), end(end), senttime(senttime), transmits(transmits) {}
    };
    typedef std::list<TCPSegmentTransmitInfo> TCPSegmentTransmitInfoItems;
    TCPSegmentTransmitInfoItems   regions;   // region[i].end == region[i+1].beg

  public:
    void set(uint32_t beg, uint32_t end, simtime_t sentTime);   // [beg,end)
    bool get(uint32_t seq, simtime_t &sentTimeOut, int &transmitsOut);
    void clearTo(uint32_t endseg);
};

#endif

