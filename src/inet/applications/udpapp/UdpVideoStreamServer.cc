//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Based on the video streaming app of the similar name by Johnny Lai.
//

#include "inet/applications/udpapp/UdpVideoStreamServer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

namespace inet {

Define_Module(UdpVideoStreamServer);

simsignal_t UdpVideoStreamServer::reqStreamBytesSignal = registerSignal("reqStreamBytes");

inline std::ostream& operator<<(std::ostream& out, const UdpVideoStreamServer::VideoStreamData& d)
{
    out << "client=" << d.clientAddr << ":" << d.clientPort
        << "  size=" << d.videoSize << "  pksent=" << d.numPkSent << "  bytesleft=" << d.bytesLeft;
    return out;
}

UdpVideoStreamServer::~UdpVideoStreamServer()
{
    for (auto& elem : streams)
        cancelAndDelete(elem.second.timer);
}

void UdpVideoStreamServer::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
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

void UdpVideoStreamServer::finish()
{
}

void UdpVideoStreamServer::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        // timer for a particular video stream expired, send packet
        sendStreamData(msg);
    }
    else
        socket.processMessage(msg);
}

void UdpVideoStreamServer::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet
    processStreamRequest(packet);
}

void UdpVideoStreamServer::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void UdpVideoStreamServer::socketClosed(UdpSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void UdpVideoStreamServer::processStreamRequest(Packet *msg)
{
    // register video stream...
    cMessage *timer = new cMessage("VideoStreamTmr");
    VideoStreamData *d = &streams[timer->getId()];
    d->timer = timer;
    d->clientAddr = msg->getTag<L3AddressInd>()->getSrcAddress();
    d->clientPort = msg->getTag<L4PortInd>()->getSrcPort();
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

void UdpVideoStreamServer::sendStreamData(cMessage *timer)
{
    auto it = streams.find(timer->getId());
    if (it == streams.end())
        throw cRuntimeError("Model error: Stream not found for timer");

    VideoStreamData *d = &(it->second);

    // generate and send a packet
    Packet *pkt = new Packet("VideoStrmPk");
    long pktLen = *packetLen;

    if (pktLen > d->bytesLeft)
        pktLen = d->bytesLeft;
    const auto& payload = makeShared<ByteCountChunk>(B(pktLen));
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    pkt->insertAtBack(payload);

    emit(packetSentSignal, pkt);
    socket.sendTo(pkt, d->clientAddr, d->clientPort);

    d->bytesLeft -= pktLen;
    d->numPkSent++;
    numPkSent++;

    // reschedule timer if there's bytes left to send
    if (d->bytesLeft > 0) {
        simtime_t interval = (*sendInterval);
        scheduleAfter(interval, timer);
    }
    else {
        streams.erase(it);
        delete timer;
    }
}

void UdpVideoStreamServer::clearStreams()
{
    for (auto& elem : streams)
        cancelAndDelete(elem.second.timer);
    streams.clear();
}

void UdpVideoStreamServer::handleStartOperation(LifecycleOperation *operation)
{
    socket.setOutputGate(gate("socketOut"));
    socket.setCallback(this);
    socket.bind(localPort);

    int timeToLive = par("timeToLive");
    if (timeToLive != -1)
        socket.setTimeToLive(timeToLive);

    int dscp = par("dscp");
    if (dscp != -1)
        socket.setDscp(dscp);

    int tos = par("tos");
    if (tos != -1)
        socket.setTos(tos);
}

void UdpVideoStreamServer::handleStopOperation(LifecycleOperation *operation)
{
    clearStreams();
    socket.setCallback(nullptr);
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void UdpVideoStreamServer::handleCrashOperation(LifecycleOperation *operation)
{
    clearStreams();
    if (operation->getRootModule() != getContainingNode(this)) // closes socket when the application crashed only
        socket.destroy(); // TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
    socket.setCallback(nullptr);
}

} // namespace inet

