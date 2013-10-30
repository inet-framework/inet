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

simsignal_t UDPVideoStreamSvr::reqStreamBytesSignal = registerSignal("reqStreamBytes");
simsignal_t UDPVideoStreamSvr::sentPkSignal = registerSignal("sentPk");

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
    for(VideoStreamMap::iterator it = streams.begin(); it  != streams.end(); ++it)
        cancelAndDelete(it->second.timer);
}

void UDPVideoStreamSvr::initialize(int stage)
{
    AppBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        sendInterval = &par("sendInterval");
        packetLen = &par("packetLen");
        videoSize = &par("videoSize");
        localPort = par("localPort");

        // statistics
        numStreams = 0;
        numPkSent = 0;

        WATCH_MAP(streams);
    }
}

void UDPVideoStreamSvr::finish()
{
}

void UDPVideoStreamSvr::handleMessageWhenUp(cMessage *msg)
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

    cMessage *timer = new cMessage("VideoStreamTmr");
    VideoStreamData *d = &streams[timer->getId()];
    d->timer = timer;
    d->clientAddr = ctrl->getSrcAddr();
    d->clientPort = ctrl->getSrcPort();
    d->videoSize = (*videoSize);
    d->bytesLeft = d->videoSize;
    d->numPkSent = 0;
    ASSERT(d->videoSize > 0);
    delete msg;

    numStreams++;
    emit(reqStreamBytesSignal, d->videoSize);

    // ... then transmit first packet right away
    sendStreamData(timer);
}

void UDPVideoStreamSvr::sendStreamData(cMessage *timer)
{
    VideoStreamMap::iterator it = streams.find(timer->getId());
    if (it == streams.end())
        throw cRuntimeError("Model error: Stream not found for timer");

    VideoStreamData *d = &(it->second);

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
    if (d->bytesLeft > 0)
    {
        simtime_t interval = (*sendInterval);
        scheduleAt(simTime()+interval, timer);
    }
    else
    {
        streams.erase(it);
        delete timer;
    }
}

void UDPVideoStreamSvr::clearStreams()
{
    for(VideoStreamMap::iterator it = streams.begin(); it  != streams.end(); ++it)
        cancelAndDelete(it->second.timer);
    streams.clear();
}

bool UDPVideoStreamSvr::startApp(IDoneCallback *doneCallback)
{
    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);

    return true;
}

bool UDPVideoStreamSvr::stopApp(IDoneCallback *doneCallback)
{
    clearStreams();
    //TODO if(socket.isOpened()) socket.close();
    return true;
}

bool UDPVideoStreamSvr::crashApp(IDoneCallback *doneCallback)
{
    clearStreams();
    return true;
}

