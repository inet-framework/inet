//
// Copyright (C) 2005 Andras Varga
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


//
// based on the video streaming app of the similar name by Johnny Lai
//


#include "UDPVideoStreamSvr.h"
#include "UDPControlInfo_m.h"


Define_Module(UDPVideoStreamSvr);

inline std::ostream& operator<<(std::ostream& out, const UDPVideoStreamSvr::VideoStreamData& d) {
    out << "client=" << d.clientAddr << ":" << d.clientPort
        << "  size=" << d.videoSize << "  pksent=" << d.numPkSent << "  bytesleft=" << d.bytesLeft;
    return out;
}

UDPVideoStreamSvr::UDPVideoStreamSvr()
{
}

UDPVideoStreamSvr::~UDPVideoStreamSvr()
{
    for (unsigned int i=0; i<streamVector.size(); i++)
        delete streamVector[i];
}

void UDPVideoStreamSvr::initialize()
{
    waitInterval = &par("waitInterval");
    packetLen = &par("packetLen");
    videoSize = &par("videoSize");
    serverPort = par("serverPort");

    numStreams = 0;
    numPkSent = 0;

    WATCH_PTRVECTOR(streamVector);

    bindToPort(serverPort);
}

void UDPVideoStreamSvr::finish()
{
    recordScalar("streams served", numStreams);
    recordScalar("packets sent", numPkSent);
}

void UDPVideoStreamSvr::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        // timer for a particular video stream expired, send packet
        sendStreamData(msg);
    }
    else
    {
        // start streaming
        processStreamRequest(msg);
    }
}


void UDPVideoStreamSvr::processStreamRequest(cMessage *msg)
{
    // register video stream...
    UDPControlInfo *ctrl = check_and_cast<UDPControlInfo *>(msg->getControlInfo());

    VideoStreamData *d = new VideoStreamData;
    d->clientAddr = ctrl->getSrcAddr();
    d->clientPort = ctrl->getSrcPort();
    d->videoSize = (*videoSize);
    d->bytesLeft = d->videoSize;
    d->numPkSent = 0;
    streamVector.push_back(d);

    cMessage *timer = new cMessage("VideoStreamTmr");
    timer->setContextPointer(d);

    // ... then transmit first packet right away
    sendStreamData(timer);

    numStreams++;
}


void UDPVideoStreamSvr::sendStreamData(cMessage *timer)
{
    VideoStreamData *d = (VideoStreamData *) timer->getContextPointer();

    // generate and send a packet
    cPacket *pkt = new cPacket("VideoStrmPk");
    long pktLen = packetLen->longValue();
    if (pktLen > d->bytesLeft)
        pktLen = d->bytesLeft;
    pkt->setByteLength(pktLen);
    sendToUDP(pkt, serverPort, d->clientAddr, d->clientPort);

    d->bytesLeft -= pktLen;
    d->numPkSent++;
    numPkSent++;

    // reschedule timer if there's bytes left to send
    if (d->bytesLeft!=0)
    {
        simtime_t interval = (*waitInterval);
        scheduleAt(simTime()+interval, timer);
    }
    else
    {
        delete timer;
        // TBD find VideoStreamData in streamVector and delete it
    }
}
