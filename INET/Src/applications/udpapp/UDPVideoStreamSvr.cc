//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
    UDPControlInfo *ctrl = check_and_cast<UDPControlInfo *>(msg->controlInfo());

    VideoStreamData *d = new VideoStreamData;
    d->clientAddr = ctrl->srcAddr();
    d->clientPort = ctrl->srcPort();
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
    VideoStreamData *d = (VideoStreamData *) timer->contextPointer();

    // generate and send a packet
    cMessage *pkt = new cMessage("VideoStrmPk");
    int pktLen = (*packetLen);
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
        double interval = (*waitInterval);
        scheduleAt(simTime()+interval, timer);
    }
    else
    {
        delete timer;
        // TBD find VideoStreamData in streamVector and delete it
    }
}
