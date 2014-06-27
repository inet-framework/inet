//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2012 Alfonso Ariza
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
#include "UDPVideoStreamSvr2.h"
#include "InterfaceTable.h"

#include "UDPControlInfo_m.h"
#include "VideoPacket_m.h"
#include "ModuleAccess.h"


Define_Module(UDPVideoStreamSvr2);

simsignal_t UDPVideoStreamSvr2::reqStreamBytesSignal = registerSignal("reqStreamBytes");
simsignal_t UDPVideoStreamSvr2::sentPkSignal = registerSignal("sentPk");

inline std::ostream& operator<<(std::ostream& out, const UDPVideoStreamSvr2::VideoStreamData& d)
{
    out << "client=" << d.clientAddr << ":" << d.clientPort
        << "  size=" << d.videoSize << "  pksent=" << d.numPkSent << "  bytesleft=" << d.bytesLeft;
    return out;
}


void UDPVideoStreamSvr2::fileParser(const char *fileName)
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
        inFile >> seqNum >>  time >> frameType >> len >> YPSNR >> UPSNR >> VPSNR;
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


UDPVideoStreamSvr2::UDPVideoStreamSvr2()
{
    videoBroadcastStream = NULL;
    restartVideoBroadcast = NULL;
    outputInterfaceBroadcast = -1;
}

UDPVideoStreamSvr2::~UDPVideoStreamSvr2()
{
    clearStreams();
    trace.clear();
    if (restartVideoBroadcast)
        cancelAndDelete(restartVideoBroadcast);
    if (videoBroadcastStream)
        delete videoBroadcastStream;

}

void UDPVideoStreamSvr2::initialize(int stage)
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
        maxSizeMacro = par("maxSizeMacro").longValue();

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

void UDPVideoStreamSvr2::finish()
{
}

void UDPVideoStreamSvr2::handleMessageWhenUp(cMessage *msg)
{
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

void UDPVideoStreamSvr2::processStreamRequest(cMessage *msg)
{
    // register video stream...
    UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(msg->getControlInfo());

    for(VideoStreamMap::iterator it = streamVector.begin(); it  != streamVector.end(); ++it)
    {
        if (it->second.clientAddr == ctrl->getSrcAddr() && it->second.clientPort == ctrl->getSrcPort())
        {
            if (!it->second.fileTrace)
            {
                if (it->second.bytesLeft)
                {
                    delete msg;
                    return;
                }
            }
            else
            {
                if (it->second.traceIndex < trace.size())
                {
                    delete msg;
                    return;
                }
            }
        }
    }
    cMessage *timer = new cMessage("VideoStreamTmr");
    VideoStreamData *d = &streamVector[timer->getId()];
    d->timer = timer;
    d->clientAddr = ctrl->getSrcAddr();
    d->clientPort = ctrl->getSrcPort();
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

void UDPVideoStreamSvr2::sendStreamData(cMessage *timer)
{
    bool deleteTimer = false;

    VideoStreamMap::iterator it = streamVector.find(timer->getId());
    if (it == streamVector.end())
        throw cRuntimeError("Model error: Stream not found for timer");

    VideoStreamData *d = &(it->second);

    // generate and send a packet
    if (d->stopTime > 0 && d->stopTime < simTime())
    {
        if (videoBroadcastStream == d)
        {
           double nextStream = par("restartBroascast").doubleValue();
           if (nextStream>=0)
               scheduleAt(simTime()+nextStream,restartVideoBroadcast);
           else
           {
               delete restartVideoBroadcast;
               restartVideoBroadcast = NULL;
           }
        }
        streamVector.erase(it);
        cancelAndDelete(timer);
        return;
    }

    cPacket *pkt = new cPacket("VideoStrmPk");
    if (!d->fileTrace)
    {
        long pktLen = packetLen->longValue();

        if (pktLen > d->bytesLeft)
            pktLen = d->bytesLeft;

        pkt->setByteLength(pktLen);

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
            deleteTimer = true;
        }
    }
    else
    {
        if (macroPackets)
        {
            simtime_t tm;
            uint64_t size = 0;
            VideoPacket *videopk = NULL;
            std::vector<VideoPacket *> macroPkt;
            do{
                VideoPacket *videopk = new VideoPacket();
                videopk->setBitLength(trace[d->traceIndex].size);
                videopk->setType(trace[d->traceIndex].type);
                videopk->setSeqNum(trace[d->traceIndex].seqNum);
                size += videopk->getByteLength();
                videopk->setFrameSize(videopk->getByteLength());
                macroPkt.push_back(videopk);
                d->traceIndex++;
            } while((size + trace[d->traceIndex].size/8 < maxSizeMacro) && (d->traceIndex < trace.size()));
            videopk = NULL;

            while(!macroPkt.empty())
            {
                VideoPacket *videopkaux = macroPkt.back();
                macroPkt.pop_back();
                if (videopk)
                    videopkaux->encapsulate(videopk);
                videopk = videopkaux;
            }
            pkt->encapsulate(videopk);
        }
        else
        {
            VideoPacket *videopk = new VideoPacket();

            videopk->setBitLength(trace[d->traceIndex].size);
            videopk->setType(trace[d->traceIndex].type);
            videopk->setSeqNum(trace[d->traceIndex].seqNum);
            videopk->setFrameSize(videopk->getByteLength());
            pkt->encapsulate(videopk);
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
        UDPSocket::SendOptions options;
        options.outInterfaceId = broadcastInterface();
        socket.sendTo(pkt, d->clientAddr, d->clientPort, &options);
    }

    if (deleteTimer)
    {
        streamVector.erase(it);
        delete timer;
    }
}

void UDPVideoStreamSvr2::broadcastVideo()
{
    // register video stream...
    if (videoBroadcastStream)
        return;
    videoBroadcastStream = new VideoStreamData;
    videoBroadcastStream->clientAddr = IPv4Address::ALLONES_ADDRESS;
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

int UDPVideoStreamSvr2::broadcastInterface()
{
    if (outputInterfaceBroadcast > 0)
        return outputInterfaceBroadcast;
    if (strcmp(par("broadcastInterface").stringValue(), "") != 0)
    {
        IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        const char *ports = par("broadcastInterface");
        InterfaceEntry *ie = ift->getInterfaceByName(ports);
        if (ie == NULL)
            throw cRuntimeError(this, "Invalid output interface name : %s", ports);
        outputInterfaceBroadcast = ie->getInterfaceId();
    }
    return outputInterfaceBroadcast;
}


void UDPVideoStreamSvr2::clearStreams()
{
    for(VideoStreamMap::iterator it = streamVector.begin(); it  != streamVector.end(); ++it)
        cancelAndDelete(it->second.timer);
    streamVector.clear();
}

bool UDPVideoStreamSvr2::handleNodeStart(IDoneCallback *doneCallback)
{
    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);

    return true;
}

bool UDPVideoStreamSvr2::handleNodeShutdown(IDoneCallback *doneCallback)
{
    clearStreams();
    //TODO if(socket.isOpened()) socket.close();
    return true;
}

void UDPVideoStreamSvr2::handleNodeCrash()
{
    clearStreams();
}
