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
   - in SctpAssociation.h an entry has to be added to
    enum SctpStreamSchedulers.
   - in SctpAssociationBase.cc in the contructor for SctpAssociation
    the new functions have to be assigned. Compare the entries
    for ROUND_ROBIN.


************************************************************/

#include <list>
#include <math.h>

#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"

namespace inet {
namespace sctp {

void SctpAssociation::initStreams(uint32 inStreams, uint32 outStreams)
{
    uint32 i;

    EV_INFO << "initStreams instreams=" << inStreams << "  outstream=" << outStreams << "\n";
    if (receiveStreams.size() == 0 && sendStreams.size() == 0) {
        for (i = 0; i < inStreams; i++) {
            SctpReceiveStream *rcvStream = new SctpReceiveStream(this);

            this->receiveStreams[i] = rcvStream;
            rcvStream->setStreamId(i);
            this->state->numMsgsReq[i] = 0;
        }
        for (i = 0; i < outStreams; i++) {
            SctpSendStream *sendStream = new SctpSendStream(this, i);
            sendStream->setStreamId(i);
            this->sendStreams[i] = sendStream;
        }
    }
}

void SctpAssociation::addInStreams(uint32 inStreams)
{
    uint32 i, j;
    char vectorName[128];
    EV_INFO << "Add " << inStreams << " inbound streams" << endl;
    for (i = receiveStreams.size(), j = 0; j < inStreams; i++, j++) {
        SctpReceiveStream *rcvStream = new SctpReceiveStream(this);
        this->receiveStreams[i] = rcvStream;
        rcvStream->setStreamId(i);
        this->state->numMsgsReq[i] = 0;
        snprintf(vectorName, sizeof(vectorName), "Stream %d Throughput", i);
        streamThroughputVectors[i] = new cOutVector(vectorName);
    }
}

void SctpAssociation::addOutStreams(uint32 outStreams)
{
    uint32 i, j;
    EV_INFO << "Add " << outStreams << " outbound streams" << endl;
    for (i = sendStreams.size(), j = 0; j < outStreams; i++, j++) {
        SctpSendStream *sendStream = new SctpSendStream(this, i);
        sendStream->setStreamId(i);
        this->sendStreams[i] = sendStream;
    }
}

void SctpAssociation::deleteStreams()
{
    for (auto & elem : sendStreams) {
        delete elem.second;
    }
    for (auto & elem : receiveStreams) {
        delete elem.second;
    }
}

int32 SctpAssociation::streamScheduler(SctpPathVariables *path, bool peek)    //peek indicates that no data is sent, but we just want to peek
{
    int32 sid, testsid;

    EV_INFO << "Stream Scheduler: RoundRobin\n";

    sid = -1;

    if ((state->ssLastDataChunkSizeSet == false || state->ssNextStream == false) &&
        (sendStreams.find(state->lastStreamScheduled)->second->getUnorderedStreamQ()->getLength() > 0 ||
         sendStreams.find(state->lastStreamScheduled)->second->getStreamQ()->getLength() > 0)) {
        sid = state->lastStreamScheduled;
        EV_DETAIL << "Stream Scheduler: again sid " << sid << ".\n";
        state->ssNextStream = true;
    } else {
        testsid = state->lastStreamScheduled;

        do {
            testsid = (testsid + 1) % outboundStreams;

            if (streamIsPending(testsid)) {
                continue;
            }

            if (sendStreams.find(testsid)->second->getUnorderedStreamQ()->getLength() > 0 ||
                sendStreams.find(testsid)->second->getStreamQ()->getLength() > 0)
            {
                sid = testsid;
                EV_DETAIL << "Stream Scheduler: chose sid " << sid << ".\n";

                if (!peek) {
                    state->lastStreamScheduled = sid;
                    break;
                }
            }
        } while (sid == -1 && testsid != (int32)state->lastStreamScheduled);
    }

    EV_INFO << "streamScheduler sid=" << sid << " lastStream=" << state->lastStreamScheduled << " outboundStreams=" << outboundStreams << " next=" << state->ssNextStream << "\n";

    if (sid >= 0 && !peek)
        state->ssLastDataChunkSizeSet = false;

    return sid;
}

int32 SctpAssociation::numUsableStreams(void)
{
    int32 count = 0;

    for (auto & elem : sendStreams)
        if (elem.second->getStreamQ()->getLength() > 0 || elem.second->getUnorderedStreamQ()->getLength() > 0) {
            count++;
        }
    return count;
}

int32 SctpAssociation::streamSchedulerRoundRobinPacket(SctpPathVariables *path, bool peek)    //peek indicates that no data is sent, but we just want to peek
{
    int32 sid, testsid, lastsid = state->lastStreamScheduled;

    EV_INFO << "Stream Scheduler: RoundRobinPacket (peek: " << peek << ")" << endl;

    sid = -1;

    if (state->ssNextStream) {
        testsid = state->lastStreamScheduled;

        do {
            testsid = (testsid + 1) % outboundStreams;

            if (streamIsPending(testsid)) {
                continue;
            }

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

int32 SctpAssociation::streamSchedulerRandom(SctpPathVariables *path, bool peek)    //peek indicates that no data is sent, but we just want to peek
{
    EV_INFO << "Stream Scheduler: Random (peek: " << peek << ")" << endl;
    state->ssNextStream = true;
    return streamSchedulerRandomPacket(path, peek);
}

int32 SctpAssociation::streamSchedulerRandomPacket(SctpPathVariables *path, bool peek)    //peek indicates that no data is sent, but we just want to peek
{
    int32 sid = -1, rnd;
    uint32 lastsid = state->lastStreamScheduled;
    std::vector<uint32> SctpWaitingSendStreamsList;

    EV_INFO << "Stream Scheduler: RandomPacket (peek: " << peek << ")" << endl;

    if (state->ssNextStream) {
        for (auto & elem : sendStreams) {
            if (elem.second->getUnorderedStreamQ()->getLength() > 0 ||
                elem.second->getStreamQ()->getLength() > 0)
            {
                SctpWaitingSendStreamsList.push_back(elem.first);
                EV_DETAIL << "Stream Scheduler: add sid " << elem.first << " to list of waiting streams.\n";
            }
        }
        if (SctpWaitingSendStreamsList.size() > 0) {
            rnd = (int)(ceil(RNGCONTEXT uniform(1, SctpWaitingSendStreamsList.size()) - 0.5));
            EV_DETAIL << "Stream Scheduler: go to " << rnd << ". element of waiting stream list.\n";
            sid = SctpWaitingSendStreamsList[rnd - 1];
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

int32 SctpAssociation::streamSchedulerPriority(SctpPathVariables *path, bool peek)    //peek indicates that no data is sent, but we just want to peek
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

int32 SctpAssociation::streamSchedulerFairBandwidth(SctpPathVariables *path, bool peek)    //peek indicates that no data is sent, but we just want to peek
{
    EV_INFO << "Stream Scheduler: FairBandwidth (peek: " << peek << ")" << endl;
    state->ssNextStream = true;
    return streamSchedulerFairBandwidthPacket(path, peek);
}

int32 SctpAssociation::streamSchedulerFairBandwidthPacket(SctpPathVariables *path, bool peek)    //peek indicates that no data is sent, but we just want to peek
{
    uint32 bandwidth = 0, packetsize = 0, lastDataChunkSize;
    int32 sid = -1;
    std::map<uint16, int32> peekMap;
    std::map<uint16, int32> *mapPointer = &(state->ssFairBandwidthMap);

    EV_INFO << "Stream Scheduler: FairBandwidthPacket (peek: " << peek << ")" << endl;

    if (state->ssFairBandwidthMap.empty()) {
        for (auto & elem : sendStreams) {
            state->ssFairBandwidthMap[elem.first] = -1;
            EV_DETAIL << "initialize sid " << elem.first << " in fb map." << endl;
        }
    }

    if (peek) {
        EV_DETAIL << "just peeking, use duplicate fb map." << endl;
        for (auto & elem : state->ssFairBandwidthMap) {
            peekMap[elem.first] = elem.second;
        }
        mapPointer = &peekMap;
    }

    lastDataChunkSize = (*mapPointer)[state->lastStreamScheduled];

    for (auto & elem : sendStreams) {
        /* There is data in this stream */
        if (elem.second->getUnorderedStreamQ()->getLength() > 0 || elem.second->getStreamQ()->getLength() > 0) {
            /* Get size of the first packet in stream */
            if (elem.second->getUnorderedStreamQ()->getLength() > 0) {
                packetsize = check_and_cast<SctpSimpleMessage *>(check_and_cast<SctpDataMsg *>(elem.second->getUnorderedStreamQ()->front())->getEncapsulatedPacket())->getByteLength();
            }
            else if (elem.second->getStreamQ()->getLength() > 0) {
                packetsize = check_and_cast<SctpSimpleMessage *>(check_and_cast<SctpDataMsg *>(elem.second->getStreamQ()->front())->getEncapsulatedPacket())->getByteLength();
            }

            /* This stream is new to the map, so add it */
            if ((*mapPointer)[elem.first] < 0) {
                if (packetsize > 0) {
                    (*mapPointer)[elem.first] = packetsize;
                    if (!peek)
                        EV_DETAIL << "Stream Scheduler: add sid " << elem.first << " with size " << packetsize << " to fair bandwidth map.\n";
                }
            }
            /* This stream is already in the map, so update it if necessary */
            else if (state->ssLastDataChunkSizeSet) {
                /* Subtract the size of the last scheduled chunk from all streams */
                (*mapPointer)[elem.first] -= lastDataChunkSize;
                if ((*mapPointer)[elem.first] < 0)
                    (*mapPointer)[elem.first] = 0;

                /* We sent from this stream the last time, so add a new message to it */
                if (elem.first == state->lastStreamScheduled) {
                    (*mapPointer)[elem.first] += packetsize;
                    if (!peek)
                        EV_DETAIL << "Stream Scheduler: updated sid " << elem.first << " with new packet of size " << packetsize << endl;
                }

                if (!peek)
                    EV_DETAIL << "Stream Scheduler: updated sid " << elem.first << " entry to size " << (*mapPointer)[elem.first] << endl;
            }
        }
        /* There is no data in this stream, so delete it from map */
        else {
            (*mapPointer)[elem.first] = -1;
            if (!peek)
                EV_DETAIL << "Stream Scheduler: sid " << elem.first << " removed from fb map" << endl;
        }
    }

    if (!peek) {
        state->ssLastDataChunkSizeSet = false;
    }

    if (state->ssNextStream) {
        for (auto & elem : *mapPointer) {
            if ((sid < 0 || (uint32)elem.second < bandwidth) && elem.second >= 0) {
                sid = elem.first;
                bandwidth = elem.second;
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

int32 SctpAssociation::streamSchedulerFCFS(SctpPathVariables *path, bool peek)    //peek indicates that no data is sent, but we just want to peek
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
            testTime = check_and_cast<SctpDataMsg *>(sendStreams.find(testsid)->second->getUnorderedStreamQ()->front())->getEnqueuingTime();
            if (sid < 0 || oldestEnqueuing > testTime) {
                oldestEnqueuing = testTime;
                sid = testsid;
                EV_DETAIL << "Stream Scheduler: chose sid " << sid << ".\n";
            }
        }

        if (sendStreams.find(testsid)->second->getStreamQ()->getLength() > 0) {
            testTime = check_and_cast<SctpDataMsg *>(sendStreams.find(testsid)->second->getStreamQ()->front())->getEnqueuingTime();
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

int32 SctpAssociation::pathStreamSchedulerManual(SctpPathVariables *path, bool peek)
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
        cStringTokenizer prioTokenizer(sctpMain->par("streamsToPaths"));
        while (prioTokenizer.hasMoreTokens()) {
            const char *token = prioTokenizer.nextToken();
            state->ssStreamToPathMap[streamNum] = (uint32)atoi(token);
            streamNum++;
        }
        if (state->ssStreamToPathMap.empty())
            throw cRuntimeError("streamsToPaths not defined");
    }

    for (SctpPathMap::const_iterator iterator = sctpPathMap.begin(); iterator != sctpPathMap.end(); ++iterator) {
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

int32 SctpAssociation::pathStreamSchedulerMapToPath(SctpPathVariables *path, bool peek)
{
    int32 thisPath = -1;
    int32 workingPaths = 0;
    for (auto & elem : sctpPathMap) {
        SctpPathVariables *myPath = elem.second;
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
    for (SctpSendStreamMap::const_iterator iterator = sendStreams.begin();
         iterator != sendStreams.end(); iterator++)
    {
        SctpSendStream *stream = iterator->second;
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

