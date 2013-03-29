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


#include <algorithm>   // min,max

#include "TCPSegmentTransmitInfoList.h"


void TCPSegmentTransmitInfoList::set(uint32_t beg, uint32_t end, simtime_t sentTime)
{
    ASSERT(regions.empty() || (seqLE(regions.front().beg, beg) && seqLE(beg, regions.back().end)));

    if (regions.empty() || (regions.back().end == beg))
        regions.push_back(TCPSegmentTransmitInfo(beg, end, sentTime, 1));
    else if (end == regions.front().beg)
        regions.push_front(TCPSegmentTransmitInfo(beg, end, sentTime, 1));
    else
    {
        TCPSegmentTransmitInfoItems::iterator i = regions.begin();
        while (seqLE(i->end, beg))
            ++i;

        ASSERT(seqLE(i->beg, beg));
        if (beg != i->beg)
        {
            regions.insert(i, TCPSegmentTransmitInfo(i->beg, beg, i->senttime, i->transmits));
            i->beg = beg;
        }
        while (i != regions.end() && seqLE(i->end, end))
        {
            ASSERT(beg == i->beg);
////            i->senttime = sentTime;
            i->transmits++;
            beg = i->end;
            ++i;
        }

        if (beg != end)
        {
            if (i != regions.end())
            {
                ASSERT(beg == i->beg);
////            regions.insert(i, TCPSegmentTransmitInfo(beg, end, sentTime, i->transmits + 1));
                regions.insert(i, TCPSegmentTransmitInfo(beg, end, i->senttime, i->transmits + 1));
                i->beg = end;
            }
            else
                regions.push_back(TCPSegmentTransmitInfo(beg, end, sentTime, 1));
        }
    }
}

bool TCPSegmentTransmitInfoList::get(uint32_t seq, simtime_t &sentTimeOut, int &transmitsOut)
{
    for (TCPSegmentTransmitInfoItems::iterator i = regions.begin(); i != regions.end(); ++i)
    {
        if (seqLess(seq, i->beg))
            break;
        if (seqLE(i->beg, seq) && seqLess(seq, i->end))
        {
            sentTimeOut = i->senttime;
            transmitsOut = i->transmits;
            return true;
        }
    }
    sentTimeOut = -1;
    transmitsOut = 0;
    return false;
}

void TCPSegmentTransmitInfoList::clearTo(uint32_t endseq)
{
    while (!regions.empty() && seqLE(regions.front().end, endseq))
        regions.pop_front();
    if (!regions.empty() && seqLess(regions.front().beg, endseq))
        regions.front().beg = endseq;
}

