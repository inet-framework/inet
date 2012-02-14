//
// Copyright (C) 2008 Irene Ruengeler
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


/************************************************************

To add new streamSchedulers
- the appropriate functions have to be implemented, preferably
    in this file.
- in SCTPAssociation.h an entry has to be added to
    enum SCTPStreamSchedulers.
- in SCTPAssociationBase.cc in the contructor for SCTPAssociation
    the new functions have to be assigned. Compare the entries
    for ROUND_ROBIN.


 ************************************************************/

#include "SCTPAssociation.h"
#include <list>
#include <math.h>

void SCTPAssociation::initStreams(uint32 inStreams, uint32 outStreams)
{
    uint32 i;

    sctpEV3 << "initStreams instreams=" << inStreams << "  outstream=" << outStreams << "\n";
    if (receiveStreams.size()==0 && sendStreams.size()==0)
    {
        for (i=0; i<inStreams; i++)
        {
            SCTPReceiveStream* rcvStream = new SCTPReceiveStream();

            this->receiveStreams[i] = rcvStream;
            rcvStream->setStreamId(i);
            this->state->numMsgsReq[i] = 0;
        }
        for (i=0; i<outStreams; i++)
        {
            SCTPSendStream* sendStream = new SCTPSendStream(i);
            this->sendStreams[i] = sendStream;
            sendStream->setStreamId(i);
        }
    }
}


int32 SCTPAssociation::streamScheduler(bool peek) //peek indicates that no data is sent, but we just want to peek
{
    int32 sid, testsid;

    sctpEV3 << "Stream Scheduler: RoundRobin\n";

    sid = -1;

    if ((state->ssLastDataChunkSizeSet == false || state->ssNextStream == false) &&
            (sendStreams.find(state->lastStreamScheduled)->second->getUnorderedStreamQ()->length() > 0 ||
                    sendStreams.find(state->lastStreamScheduled)->second->getStreamQ()->length() > 0))
    {
        sid = state->lastStreamScheduled;
        sctpEV3 << "Stream Scheduler: again sid " << sid << ".\n";
        state->ssNextStream = true;
    }
    else
    {
        testsid = state->lastStreamScheduled;

        do {
            testsid = (testsid + 1) % outboundStreams;

            if (sendStreams.find(testsid)->second->getUnorderedStreamQ()->length() > 0 ||
                    sendStreams.find(testsid)->second->getStreamQ()->length() > 0)
            {
                sid = testsid;
                sctpEV3 << "Stream Scheduler: chose sid " << sid << ".\n";

                if (!peek)
                    state->lastStreamScheduled = sid;
            }
        } while (sid == -1 && testsid != (int32) state->lastStreamScheduled);

    }

    sctpEV3 << "streamScheduler sid=" << sid << " lastStream=" << state->lastStreamScheduled << " outboundStreams=" << outboundStreams << " next=" << state->ssNextStream << "\n";

    state->ssLastDataChunkSizeSet = false;

    return sid;
}


int32 SCTPAssociation::numUsableStreams(void)
{
    int32 count = 0;

    for (SCTPSendStreamMap::iterator iter=sendStreams.begin(); iter!=sendStreams.end(); iter++)
        if (iter->second->getStreamQ()->length()>0 || iter->second->getUnorderedStreamQ()->length()>0)
        {
            count++;
        }
    return count;
}
