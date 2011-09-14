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

simsignal_t UDPVideoStreamSvr::reqStreamBytesSignal = SIMSIGNAL_NULL;
simsignal_t UDPVideoStreamSvr::sentPkSignal = SIMSIGNAL_NULL;

inline std::ostream& operator<<(std::ostream& out, const UDPVideoStreamSvr::VideoStreamData& d)
{
    out << "client=" << d.clientAddr << ":" << d.clientPort
        << "  size=" << d.videoSize << "  pksent=" << d.numPkSent << "  bytesleft=" << d.bytesLeft;
    return out;
}

UDPVideoStreamSvr::UDPVideoStreamSvr()
{
}

UDPVideoStreamSvr::~UDPVideoStreamSvr()
{
    for (unsigned int i=0; i < streamVector.size(); i++)
        delete streamVector[i];
}

void UDPVideoStreamSvr::initialize()
{
    sendInterval = &par("sendInterval");
    packetLen = &par("packetLen");
    videoSize = &par("videoSize");
    localPort = par("localPort");

    // statistics
    numStreams = 0;
    numPkSent = 0;
    reqStreamBytesSignal = registerSignal("reqStreamBytes");
    sentPkSignal = registerSignal("sentPk");

    WATCH_PTRVECTOR(streamVector);

    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);
}

void UDPVideoStreamSvr::finish()
{
}

void UDPVideoStreamSvr::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        // timer for a particular video stream expired, send packet
        sendStreamData(msg);
    }
    else if (msg->getKind() == UDP_I_DATA)
    {
        // start streaming
        processStreamRequest(msg);
    }
    else if (msg->getKind() == UDP_I_ERROR)
    {
        EV << "Ignoring UDP error report\n";
        delete msg;
    }
    else
    {
        error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }
}

void UDPVideoStreamSvr::processStreamRequest(cMessage *msg)
{
    // register video stream...
    UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(msg->getControlInfo());

    VideoStreamData *d = new VideoStreamData;
    d->clientAddr = ctrl->getSrcAddr();
    d->clientPort = ctrl->getSrcPort();
    d->videoSize = (*videoSize);
    d->bytesLeft = d->videoSize;
    d->numPkSent = 0;
    streamVector.push_back(d);
    delete msg;

    cMessage *timer = new cMessage("VideoStreamTmr");
    timer->setContextPointer(d);

    // ... then transmit first packet right away
    sendStreamData(timer);

    numStreams++;
    emit(reqStreamBytesSignal, d->videoSize);
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
    emit(sentPkSignal, pkt);
    socket.sendTo(pkt, d->clientAddr, d->clientPort);

    d->bytesLeft -= pktLen;
    d->numPkSent++;
    numPkSent++;

    // reschedule timer if there's bytes left to send
    if (d->bytesLeft != 0)
    {
        simtime_t interval = (*sendInterval);
        scheduleAt(simTime()+interval, timer);
    }
    else
    {
        delete timer;
        // TBD find VideoStreamData in streamVector and delete it
    }
}

