//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2010 Robin Seggelmann
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

#include <list>
#include <math.h>

#include "inet/transportlayer/sctp/SCTPAssociation.h"
#include "inet/transportlayer/contract/sctp/SCTPCommand_m.h"

namespace inet {

namespace sctp {

void SCTPAssociation::initStreams(uint32 inStreams, uint32 outStreams)
{
    uint32 i;

    EV_INFO << "initStreams instreams=" << inStreams << "  outstream=" << outStreams << "\n";
    if (receiveStreams.size() == 0 && sendStreams.size() == 0) {
        for (i = 0; i < inStreams; i++) {
            SCTPReceiveStream *rcvStream = new SCTPReceiveStream();

            this->receiveStreams[i] = rcvStream;
            rcvStream->setStreamId(i);
            this->state->numMsgsReq[i] = 0;
        }
        for (i = 0; i < outStreams; i++) {
            SCTPSendStream *sendStream = new SCTPSendStream(i);
            sendStream->setStreamId(i);
            this->sendStreams[i] = sendStream;
        }
    }
}

int32 SCTPAssociation::streamScheduler(SCTPPathVariables *path, bool peek)    //peek indicates that no data is sent, but we just want to peek
{
    int32 sid, testsid;

    EV_INFO << "Stream Scheduler: RoundRobin\n";

    sid = -1;

    if ((state->ssLastDataChunkSizeSet == false || state->ssNextStream == false) &&
        (sendStreams.find(state->lastStreamScheduled)->second->getUnorderedStreamQ()->getLength() > 0 ||
         sendStreams.find(state->lastStreamScheduled)->second->getStreamQ()->getLength() > 0))
    {
        sid = state->lastStreamScheduled;
        EV_DETAIL << "Stream Scheduler: again sid " << sid << ".\n";
        state->ssNextStream = true;
    }
    else {
        testsid = state->lastStreamScheduled;

        do {
            testsid = (testsid + 1) % outboundStreams;

            if (sendStreams.find(testsid)->second->getUnorderedStreamQ()->getLength() > 0 ||
                sendStreams.find(testsid)->second->getStreamQ()->getLength() > 0)
            {
                sid = testsid;
                EV_DETAIL << "Stream Scheduler: chose sid " << sid << ".\n";

                if (!peek)
                    state->lastStreamScheduled = sid;
            }
        } while (sid == -1 && testsid != (int32)state->lastStreamScheduled);
    }

    EV_INFO << "streamScheduler sid=" << sid << " lastStream=" << state->lastStreamScheduled << " outboundStreams=" << outboundStreams << " next=" << state->ssNextStream << "\n";

    if (sid >= 0 && !peek)
        state->ssLastDataChunkSizeSet = false;

    return sid;
}

int32 SCTPAssociation::numUsableStreams(void)
{
    int32 count = 0;

    for (auto iter = sendStreams.begin(); iter != sendStreams.end(); iter++)
        if (iter->second->getStreamQ()->getLength() > 0 || iter->second->getUnorderedStreamQ()->getLength() > 0) {
            count++;
        }
    return count;
}

int32 SCTPAssociation::streamSchedulerRoundRobinPacket(SCTPPathVariables *path, bool peek)    //peek indicates that no data is sent, but we just want to peek
{
    int32 sid, testsid, lastsid = state->lastStreamScheduled;

    EV_INFO << "Stream Scheduler: RoundRobinPacket (peek: " << peek << ")" << endl;

    sid = -1;

    if (state->ssNextStream) {
        testsid = state->lastStreamScheduled;

        do {
            testsid = (testsid + 1) % outboundStreams;

            if (sendStreams.find(testsid)->second->getUnorderedStreamQ()->getLength() > 0 ||
                sendStreams.find(testsid)->second->getStreamQ()->getLength() > 0)
            {
                sid = testsid;
                EV_DETAIL << "Stream Scheduler: chose sid " << sid << ".\n";

                if (!peek)
                    state->lastStreamScheduled = sid;
            }
        } while (sid == -1 && testsid != (int32)state->lastStreamScheduled);
    }
    else {
        if (sendStreams.find(state->lastStreamScheduled)->second->getUnorderedStreamQ()->getLength() > 0 ||
            sendStreams.find(state->lastStreamScheduled)->second->getStreamQ()->getLength() > 0)
        {
            sid = state->lastStreamScheduled;
            EV_DETAIL << "Stream Scheduler: again sid " << sid << ".\n";
        }
    }

    EV_INFO << "streamScheduler sid=" << sid << " lastStream=" << lastsid << " outboundStreams=" << outboundStreams << " next=" << state->ssNextStream << "\n";

    if (sid >= 0 && !peek)
        state->ssNextStream = false;

    return sid;
}

int32 SCTPAssociation::streamSchedulerRandom(SCTPPathVariables *path, bool peek)    //peek indicates that no data is sent, but we just want to peek
{
    EV_INFO << "Stream Scheduler: Random (peek: " << peek << ")" << endl;
    state->ssNextStream = true;
    return streamSchedulerRandomPacket(path, peek);
}

int32 SCTPAssociation::streamSchedulerRandomPacket(SCTPPathVariables *path, bool peek)    //peek indicates that no data is sent, but we just want to peek
{
    int32 sid = -1, rnd;
    uint32 lastsid = state->lastStreamScheduled;
    std::vector<uint32> SCTPWaitingSendStreamsList;

    EV_INFO << "Stream Scheduler: RandomPacket (peek: " << peek << ")" << endl;

    if (state->ssNextStream) {
        for (auto iter = sendStreams.begin(); iter != sendStreams.end(); ++iter) {
            if (iter->second->getUnorderedStreamQ()->getLength() > 0 ||
                iter->second->getStreamQ()->getLength() > 0)
            {
                SCTPWaitingSendStreamsList.push_back(iter->first);
                EV_DETAIL << "Stream Scheduler: add sid " << iter->first << " to list of waiting streams.\n";
            }
        }
        if (SCTPWaitingSendStreamsList.size() > 0) {
            rnd = (int)(ceil(RNGCONTEXT uniform(1, SCTPWaitingSendStreamsList.size()) - 0.5));
            EV_DETAIL << "Stream Scheduler: go to " << rnd << ". element of waiting stream list.\n";
            sid = SCTPWaitingSendStreamsList[rnd - 1];
            if (!peek)
                state->lastStreamScheduled = sid;
            EV_DETAIL << "Stream Scheduler: chose sid " << sid << ".\n";
        }
    }
    else {
        if (sendStreams.find(state->lastStreamScheduled)->second->getUnorderedStreamQ()->getLength() > 0 ||
            sendStreams.find(state->lastStreamScheduled)->second->getStreamQ()->getLength() > 0)
        {
            sid = state->lastStreamScheduled;
            EV_DETAIL << "Stream Scheduler: again sid " << sid << ".\n";
        }
    }

    EV_INFO << "streamScheduler sid=" << sid << " lastStream=" << lastsid << " outboundStreams=" << outboundStreams << " next=" << state->ssNextStream << "\n";

    if (sid >= 0 && !peek)
        state->ssNextStream = false;

    return sid;
}

int32 SCTPAssociation::streamSchedulerPriority(SCTPPathVariables *path, bool peek)    //peek indicates that no data is sent, but we just want to peek
{
    int32 sid = 0, testsid;
    std::list<uint32> PriorityList;
    std::list<uint32> StreamList;

    EV_INFO << "Stream Scheduler: Priority (peek: " << peek << ")" << endl;

    sid = -1;

    if ((state->ssLastDataChunkSizeSet == false && state->ssNextStream == true) &&
        (sendStreams.find(state->lastStreamScheduled)->second->getUnorderedStreamQ()->getLength() > 0 ||
         sendStreams.find(state->lastStreamScheduled)->second->getStreamQ()->getLength() > 0))
    {
        sid = state->lastStreamScheduled;
        EV_DETAIL << "Stream Scheduler: again sid " << sid << ".\n";

        return sid;
    }

    if (!state->ssNextStream) {
        testsid = -1;
        state->ssNextStream = true;
    }
    else
        testsid = state->lastStreamScheduled;

    do {
        testsid = (testsid + 1) % outboundStreams;

        if (sendStreams.find(testsid)->second->getUnorderedStreamQ()->getLength() > 0 ||
            sendStreams.find(testsid)->second->getStreamQ()->getLength() > 0)
        {
            if (sid < 0 || state->ssPriorityMap[testsid] < state->ssPriorityMap[sid]) {
                sid = testsid;
                EV_DETAIL << "Stream Scheduler: chose sid " << sid << ".\n";
            }
        }
    } while (testsid != (int32)state->lastStreamScheduled);

    if (!peek && sid >= 0) {
        state->lastStreamScheduled = sid;
        state->ssLastDataChunkSizeSet = false;
    }

    return sid;
}

int32 SCTPAssociation::streamSchedulerFairBandwidth(SCTPPathVariables *path, bool peek)    //peek indicates that no data is sent, but we just want to peek
{
    EV_INFO << "Stream Scheduler: FairBandwidth (peek: " << peek << ")" << endl;
    state->ssNextStream = true;
    return streamSchedulerFairBandwidthPacket(path, peek);
}

int32 SCTPAssociation::streamSchedulerFairBandwidthPacket(SCTPPathVariables *path, bool peek)    //peek indicates that no data is sent, but we just want to peek
{
    uint32 bandwidth = 0, packetsize = 0, lastDataChunkSize;
    int32 sid = -1;
    std::map<uint16, int32> peekMap;
    std::map<uint16, int32> *mapPointer = &(state->ssFairBandwidthMap);

    EV_INFO << "Stream Scheduler: FairBandwidthPacket (peek: " << peek << ")" << endl;

    if (state->ssFairBandwidthMap.empty()) {
        for (auto iter = sendStreams.begin(); iter != sendStreams.end(); ++iter) {
            state->ssFairBandwidthMap[iter->first] = -1;
            EV_DETAIL << "initialize sid " << iter->first << " in fb map." << endl;
        }
    }

    if (peek) {
        EV_DETAIL << "just peeking, use duplicate fb map." << endl;
        for (auto iter = state->ssFairBandwidthMap.begin(); iter != state->ssFairBandwidthMap.end(); ++iter) {
            peekMap[iter->first] = iter->second;
        }
        mapPointer = &peekMap;
    }

    lastDataChunkSize = (*mapPointer)[state->lastStreamScheduled];

    for (auto iter = sendStreams.begin(); iter != sendStreams.end(); ++iter) {
        /* There is data in this stream */
        if (iter->second->getUnorderedStreamQ()->getLength() > 0 || iter->second->getStreamQ()->getLength() > 0) {
            /* Get size of the first packet in stream */
            if (iter->second->getUnorderedStreamQ()->getLength() > 0) {
                packetsize = check_and_cast<SCTPSimpleMessage *>(((SCTPDataMsg *)iter->second->getUnorderedStreamQ()->front())->getEncapsulatedPacket())->getByteLength();
            }
            else if (iter->second->getStreamQ()->getLength() > 0) {
                packetsize = check_and_cast<SCTPSimpleMessage *>(((SCTPDataMsg *)iter->second->getStreamQ()->front())->getEncapsulatedPacket())->getByteLength();
            }

            /* This stream is new to the map, so add it */
            if ((*mapPointer)[iter->first] < 0) {
                if (packetsize > 0) {
                    (*mapPointer)[iter->first] = packetsize;
                    if (!peek)
                        EV_DETAIL << "Stream Scheduler: add sid " << iter->first << " with size " << packetsize << " to fair bandwidth map.\n";
                }
            }
            /* This stream is already in the map, so update it if necessary */
            else if (state->ssLastDataChunkSizeSet) {
                /* Subtract the size of the last scheduled chunk from all streams */
                (*mapPointer)[iter->first] -= lastDataChunkSize;
                if ((*mapPointer)[iter->first] < 0)
                    (*mapPointer)[iter->first] = 0;

                /* We sent from this stream the last time, so add a new message to it */
                if (iter->first == state->lastStreamScheduled) {
                    (*mapPointer)[iter->first] += packetsize;
                    if (!peek)
                        EV_DETAIL << "Stream Scheduler: updated sid " << iter->first << " with new packet of size " << packetsize << endl;
                }

                if (!peek)
                    EV_DETAIL << "Stream Scheduler: updated sid " << iter->first << " entry to size " << (*mapPointer)[iter->first] << endl;
            }
        }
        /* There is no data in this stream, so delete it from map */
        else {
            (*mapPointer)[iter->first] = -1;
            if (!peek)
                EV_DETAIL << "Stream Scheduler: sid " << iter->first << " removed from fb map" << endl;
        }
    }

    if (!peek) {
        state->ssLastDataChunkSizeSet = false;
    }

    if (state->ssNextStream) {
        for (auto iter = mapPointer->begin(); iter != mapPointer->end(); ++iter) {
            if ((sid < 0 || (uint32)iter->second < bandwidth) && iter->second >= 0) {
                sid = iter->first;
                bandwidth = iter->second;
                if (!peek)
                    EV_DETAIL << "Stream Scheduler: chose sid " << sid << ".\n";
            }
        }
    }
    else {
        if (sendStreams.find(state->lastStreamScheduled)->second->getUnorderedStreamQ()->getLength() > 0 ||
            sendStreams.find(state->lastStreamScheduled)->second->getStreamQ()->getLength() > 0)
        {
            sid = state->lastStreamScheduled;
            if (!peek)
                EV_DETAIL << "Stream Scheduler: again sid " << sid << ".\n";
        }
    }

    if (sid >= 0 && !peek) {
        state->lastStreamScheduled = sid;
        state->ssNextStream = false;
    }

    return sid;
}

int32 SCTPAssociation::streamSchedulerFCFS(SCTPPathVariables *path, bool peek)    //peek indicates that no data is sent, but we just want to peek
{
    int32 sid, testsid;
    simtime_t oldestEnqueuing, testTime;

    EV_INFO << "Stream Scheduler: First-come, first-serve (peek: " << peek << ")" << endl;

    sid = -1;

    if (!state->ssNextStream) {
        testsid = -1;
        state->ssNextStream = true;
    }
    else
        testsid = state->lastStreamScheduled;

    do {
        testsid = (testsid + 1) % outboundStreams;

        if (sendStreams.find(testsid)->second->getUnorderedStreamQ()->getLength() > 0) {
            testTime = ((SCTPDataMsg *)sendStreams.find(testsid)->second->getUnorderedStreamQ()->front())->getEnqueuingTime();
            if (sid < 0 || oldestEnqueuing > testTime) {
                oldestEnqueuing = testTime;
                sid = testsid;
                EV_DETAIL << "Stream Scheduler: chose sid " << sid << ".\n";
            }
        }

        if (sendStreams.find(testsid)->second->getStreamQ()->getLength() > 0) {
            testTime = ((SCTPDataMsg *)sendStreams.find(testsid)->second->getStreamQ()->front())->getEnqueuingTime();
            if (sid < 0 || oldestEnqueuing > testTime) {
                oldestEnqueuing = testTime;
                sid = testsid;
                EV_DETAIL << "Stream Scheduler: chose sid " << sid << ".\n";
            }
        }
    } while (testsid != (int32)state->lastStreamScheduled);

    if (!peek && sid >= 0)
        state->lastStreamScheduled = sid;

    return sid;
}

int32 SCTPAssociation::pathStreamSchedulerManual(SCTPPathVariables *path, bool peek)
{
    uint32 pathNo = 0;
    int32 testsid, sid = -1;
    uint32 lastsid = state->lastStreamScheduled;

    EV_INFO << "Stream Scheduler: path-aware Manual (peek: " << peek << ")" << endl;

    if (state->ssStreamToPathMap.empty()) {
        for (uint16 str = 0; str < outboundStreams; str++) {
            state->ssStreamToPathMap[str] = 0;
        }

        // Fill Stream to Path map
        uint16 streamNum = 0;
        cStringTokenizer prioTokenizer(sctpMain->par("streamsToPaths").stringValue());
        while (prioTokenizer.hasMoreTokens()) {
            const char *token = prioTokenizer.nextToken();
            state->ssStreamToPathMap[streamNum] = (uint32)atoi(token);
            streamNum++;
        }
        if (state->ssStreamToPathMap.empty())
            throw cRuntimeError("streamsToPaths not defined");
    }

    for (SCTPPathMap::const_iterator iterator = sctpPathMap.begin(); iterator != sctpPathMap.end(); ++iterator) {
        if (path == iterator->second)
            break;
        pathNo++;
    }
    EV_INFO << "Stream Scheduler: about to send on " << pathNo + 1 << ". path" << endl;

    testsid = state->lastStreamScheduled;

    do {
        testsid = (testsid + 1) % outboundStreams;

        if (state->ssStreamToPathMap[testsid] == (int32)pathNo) {
            sid = testsid;
            EV_DETAIL << "Stream Scheduler: chose sid " << sid << ".\n";
            if (!peek)
                state->lastStreamScheduled = sid;
        }
    } while (sid == -1 && testsid != (int32)state->lastStreamScheduled);

    EV_INFO << "streamScheduler sid=" << sid << " lastStream=" << lastsid << " outboundStreams=" << outboundStreams << " next=" << state->ssNextStream << "\n";

    return sid;
}

int32 SCTPAssociation::pathStreamSchedulerMapToPath(SCTPPathVariables *path, bool peek)
{
    int32 thisPath = -1;
    int32 workingPaths = 0;
    for (auto iterator = sctpPathMap.begin(); iterator != sctpPathMap.end(); ++iterator) {
        SCTPPathVariables *myPath = iterator->second;
        if (myPath->activePath) {
            if (myPath == path) {
                thisPath = workingPaths;
            }
            workingPaths++;
        }
    }
    if (thisPath == -1) {
        std::cout << "THIS PATH IS NOT WORKING???" << endl;
        return -1;
    }

    int32 sid = -1;
    for (SCTPSendStreamMap::const_iterator iterator = sendStreams.begin();
         iterator != sendStreams.end(); iterator++)
    {
        SCTPSendStream *stream = iterator->second;
        if ((stream->getStreamId() % workingPaths) == thisPath) {    // Maps to "path" ...
            if ((stream->getUnorderedStreamQ()->getLength() > 0) ||    // Stream has something to send ...
                (stream->getStreamQ()->getLength() > 0))
            {
                assert(sid == -1);
                sid = stream->getStreamId();
                break;
            }
        }
    }

    EV_INFO << "pathStreamSchedulerMapToPath sid=" << sid
            << " path=" << path->remoteAddress
            << " (path " << (1 + thisPath)
            << " of " << workingPaths << ")" << endl;
    return sid;
}

} // namespace sctp

} // namespace inet

