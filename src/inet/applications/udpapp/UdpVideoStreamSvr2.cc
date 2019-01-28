//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2015 A. Ariza (Malaga University)
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


#include <iostream>
#include <fstream>
#include "inet/networklayer/common/InterfaceTable.h"

#include "inet/applications/udpapp/VideoPacket_m.h"
#include "inet/applications/udpapp/UdpVideoStreamSvr2.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

namespace inet {

Define_Module(UdpVideoStreamSvr2);

simsignal_t UdpVideoStreamSvr2::reqStreamBytesSignal = registerSignal("reqStreamBytes");
simsignal_t UdpVideoStreamSvr2::sentPkSignal = registerSignal("sentPk");

inline std::ostream& operator<<(std::ostream& out, const UdpVideoStreamSvr2::VideoStreamData& d)
{
    out << "client=" << d.clientAddr << ":" << d.clientPort
        << "  size=" << d.videoSize << "  pksent=" << d.numPkSent << "  bytesleft=" << d.bytesLeft;
    return out;
}


void UdpVideoStreamSvr2::fileParser(const char *fileName)
{
    std::string fi(fileName);
    fi = "./"+fi;
    std::ifstream inFile(fi.c_str());

    if (!inFile)
    {
        error("Error while opening input file (File not found or incorrect type)\n");
    }

    trace.clear();
    simtime_t timedata;
    while (!inFile.eof())
    {
        int seqNum = 0;
        float time = 0;
        float YPSNR = 0;
        float UPSNR = 0;
        float VPSNR = 0;
        int len = 0;
        char frameType = 0;
        std::string line;
        std::getline (inFile,line);
        size_t pos = line.find("#");
        if (pos != std::string::npos)
        {
            if (pos == 0)
                line.clear();
            else
                line = line.substr(0,pos-1);
        }
        if (line.empty())
            continue;
        std::istringstream(line) >> seqNum >>  time >> frameType >> len >> YPSNR >> UPSNR >> VPSNR;
        // inFile >> seqNum >>  time >> frameType >> len >> YPSNR >> UPSNR >> VPSNR;
        VideoInfo info;
        info.seqNum = seqNum;
        info.type = frameType;
        info.size = len;
        info.timeFrame = time;
        // now insert in time order
        if (trace.empty() || trace.back().timeFrame < info.timeFrame)
            trace.push_back(info);
        else
        {
            // search the place
            for (int i = trace.size()-1 ; i >= 0; i--)
            {
                if (trace[i].timeFrame < info.timeFrame)
                {
                    trace.insert(trace.begin()+i+1,info);
                    break;
                }
            }
        }
    }
    inFile.close();
}


UdpVideoStreamSvr2::UdpVideoStreamSvr2()
{
    videoBroadcastStream = nullptr;
    restartVideoBroadcast = nullptr;
    outputInterfaceBroadcast = -1;
}

UdpVideoStreamSvr2::~UdpVideoStreamSvr2()
{
    clearStreams();
    trace.clear();
    if (restartVideoBroadcast)
        cancelAndDelete(restartVideoBroadcast);
    if (videoBroadcastStream)
        delete videoBroadcastStream;

}

void UdpVideoStreamSvr2::initialize(int stage)
{
    ApplicationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        sendInterval = &par("sendInterval");
        packetLen = &par("packetLen");
        videoSize = &par("videoSize");
        stopTime = &par("stopTime");
        localPort = par("localPort");

        macroPackets = par("macroPackets");
        maxSizeMacro = par("maxSizeMacro").intValue();

        if (par("videoBroadcast").boolValue())
        {
            restartVideoBroadcast = new cMessage();
            scheduleAt(par("startBroadcast").doubleValue(), restartVideoBroadcast);
        }

        // statistics
        numStreams = 0;
        numPkSent = 0;

        trace.clear();
        std::string fileName(par("traceFileName").stringValue());
        if (!fileName.empty())
            fileParser(fileName.c_str());
        WATCH_MAP(streamVector);
    }
}

void UdpVideoStreamSvr2::finish()
{
}

void UdpVideoStreamSvr2::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        // timer for a particular video stream expired, send packet
        if (msg->isSelfMessage())
        {
            if (restartVideoBroadcast && restartVideoBroadcast == msg)
                broadcastVideo();
            else
            {
                // timer for a particular video stream expired, send packet
                sendStreamData(msg);
            }
        }
    }
    else
        socket.processMessage(msg);
}


void UdpVideoStreamSvr2::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet
    processStreamRequest(packet);
}

void UdpVideoStreamSvr2::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void UdpVideoStreamSvr2::socketClosed(UdpSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void UdpVideoStreamSvr2::processStreamRequest(Packet *msg)
{
    // register video stream...
    auto clientAddr = msg->getTag<L3AddressInd>()->getSrcAddress();
    auto clientPort = msg->getTag<L4PortInd>()->getSrcPort();

    for(auto it = streamVector.begin(); it  != streamVector.end(); ++it) {
        if (it->second.clientAddr == clientAddr && it->second.clientPort == clientPort) {
            if (!it->second.fileTrace) {
                if (it->second.bytesLeft) {
                    delete msg;
                    return;
                }
            }
            else {
                if (it->second.traceIndex < trace.size()) {
                    delete msg;
                    return;
                }
            }
        }
    }
    cMessage *timer = new cMessage("VideoStreamTmr");
    VideoStreamData *d = &streamVector[timer->getId()];
    d->timer = timer;
    d->clientAddr = clientAddr;
    d->clientPort = clientPort;
    d->videoSize = (*videoSize);
    d->bytesLeft = d->videoSize;
    d->traceIndex = 0;
    d->timeInit = simTime();
    d->fileTrace = false;
    double stop = (*stopTime);
    if (stop > 0)
        d->stopTime = simTime() + stop;
    else
        d->stopTime = 0;
    d->numPkSent = 0;

    if (!trace.empty())
        d->fileTrace = true;
    delete msg;
    // ... then transmit first packet right away
    sendStreamData(timer);

    numStreams++;
    emit(reqStreamBytesSignal, d->videoSize);
}

void UdpVideoStreamSvr2::sendStreamData(cMessage *timer)
{
    bool deleteTimer = false;

    auto it = streamVector.find(timer->getId());
    if (it == streamVector.end())
        throw cRuntimeError("Model error: Stream not found for timer");

    VideoStreamData *d = &(it->second);

    // generate and send a packet
    if (d->stopTime > 0 && d->stopTime < simTime()) {
        if (videoBroadcastStream == d) {
           double nextStream = par("restartBroascast").doubleValue();
           if (nextStream>=0)
               scheduleAt(simTime()+nextStream,restartVideoBroadcast);
           else {
               delete restartVideoBroadcast;
               restartVideoBroadcast = nullptr;
           }
        }
        streamVector.erase(it);
        cancelAndDelete(timer);
        return;
    }

    Packet *pkt = new Packet("VideoStrmPk");
    if (!d->fileTrace) {
        long pktLen = packetLen->intValue();

        if (pktLen > d->bytesLeft)
            pktLen = d->bytesLeft;

        const auto& payload = makeShared<ByteCountChunk>(B(pktLen));
        payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
        pkt->insertAtBack(payload);

        d->bytesLeft -= pktLen;
        d->numPkSent++;
        numPkSent++;
        // reschedule timer if there's bytes left to send
        if (d->bytesLeft != 0) {
            simtime_t interval = (*sendInterval);
            scheduleAt(simTime()+interval, timer);
        }
        else {
            deleteTimer = true;
        }
    }
    else {
        if (macroPackets) {
            simtime_t tm;
            uint64_t size = 0;

            do{
                const auto& videopk = makeShared<VideoPacket>();
                videopk->setChunkLength(b(trace[d->traceIndex].size));
                videopk->setType(trace[d->traceIndex].type);
                videopk->setSeqNum(trace[d->traceIndex].seqNum);
                videopk->addTag<CreationTimeTag>()->setCreationTime(simTime());
                auto len = B(videopk->getChunkLength());
                videopk->setFrameSize(len);
                pkt->insertAtBack(videopk);
                d->traceIndex++;
            } while((size + trace[d->traceIndex].size/8 < maxSizeMacro) && (d->traceIndex < trace.size()));

        }
        else {
            const auto& videopk = makeShared<VideoPacket>();

            videopk->setChunkLength(b(trace[d->traceIndex].size));

            videopk->setType(trace[d->traceIndex].type);
            videopk->setSeqNum(trace[d->traceIndex].seqNum);
            auto len = B(videopk->getChunkLength());
            videopk->addTag<CreationTimeTag>()->setCreationTime(simTime());
            videopk->setFrameSize(len);
            pkt->insertAtBack(videopk);
            d->traceIndex++;
        }
        if (d->traceIndex >= trace.size())
            deleteTimer = true;
        else
            scheduleAt(d->timeInit + trace[d->traceIndex].timeFrame, timer);

    }
    emit(sentPkSignal, pkt);
    if (videoBroadcastStream != d)
        socket.sendTo(pkt, d->clientAddr, d->clientPort);
    else
    {
        socket.setMulticastOutputInterface(broadcastInterface());
        socket.sendTo(pkt, d->clientAddr, d->clientPort);
    }

    if (deleteTimer)
    {
        streamVector.erase(it);
        delete timer;
    }
}

void UdpVideoStreamSvr2::broadcastVideo()
{
    // register video stream...
    if (videoBroadcastStream)
        return;
    videoBroadcastStream = new VideoStreamData;
    videoBroadcastStream->clientAddr = Ipv4Address::ALLONES_ADDRESS;
    videoBroadcastStream->clientPort = localPort;
    videoBroadcastStream->videoSize = (*videoSize);
    videoBroadcastStream->bytesLeft = videoBroadcastStream->videoSize;
    videoBroadcastStream->traceIndex = 0;
    videoBroadcastStream->timeInit = simTime();
    videoBroadcastStream->fileTrace = false;
    double stop = (*stopTime);
    if (stop > 0)
        videoBroadcastStream->stopTime = simTime() + stop;
    else
        videoBroadcastStream->stopTime = 0;
    videoBroadcastStream->numPkSent = 0;

    if (!trace.empty())
        videoBroadcastStream->fileTrace = true;

    cMessage *timer = new cMessage("VideoStreamTmr");
    timer->setContextPointer(videoBroadcastStream);

    // ... then transmit first packet right away
    sendStreamData(timer);

    numStreams++;
    emit(reqStreamBytesSignal, videoBroadcastStream->videoSize);
}

int UdpVideoStreamSvr2::broadcastInterface()
{
    if (outputInterfaceBroadcast > 0)
        return outputInterfaceBroadcast;
    if (strcmp(par("broadcastInterface").stringValue(), "") != 0)
    {
        IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        const char *ports = par("broadcastInterface");
        InterfaceEntry *ie = ift->getInterfaceByName(ports);
        if (ie == nullptr)
            throw cRuntimeError(this, "Invalid output interface name : %s", ports);
        outputInterfaceBroadcast = ie->getInterfaceId();
    }
    return outputInterfaceBroadcast;
}


void UdpVideoStreamSvr2::clearStreams()
{
    for(auto it = streamVector.begin(); it  != streamVector.end(); ++it)
        cancelAndDelete(it->second.timer);
    streamVector.clear();
}

void UdpVideoStreamSvr2::handleStartOperation(LifecycleOperation *operation)
{
    socket.setOutputGate(gate("socketOut"));
    socket.bind(localPort);
    socket.setCallback(this);
}

void UdpVideoStreamSvr2::handleStopOperation(LifecycleOperation *operation)
{
    clearStreams();
    socket.setCallback(nullptr);
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void UdpVideoStreamSvr2::handleCrashOperation(LifecycleOperation *operation)
{
    clearStreams();
    if (operation->getRootModule() != getContainingNode(this))     // closes socket when the application crashed only
        socket.destroy();    //TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
    socket.setCallback(nullptr);
}

}
