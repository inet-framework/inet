/*
 * UdpBasicFlooding.cc
 *
 *  Created on: Jan 29, 2021
 *      Author: alfonso
 */




//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2007 Universidad de Malaga
// Copyright (C) 2011 Zoltan Bojthe
// Copyright (C) 2013 Universidad de Malaga
// Copyright (C) 2021 Universidad de Malaga
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


#include "inet/applications/udpapp/UdpBasicFlooding.h"

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"

namespace inet {

Define_Module(UdpBasicFlooding);

int UdpBasicFlooding::counter;

simsignal_t UdpBasicFlooding::sentPkSignal = registerSignal("sentPk");
simsignal_t UdpBasicFlooding::rcvdPkSignal = registerSignal("rcvdPk");
simsignal_t UdpBasicFlooding::outOfOrderPkSignal = registerSignal("outOfOrderPk");
simsignal_t UdpBasicFlooding::dropPkSignal = registerSignal("dropPk");
simsignal_t UdpBasicFlooding::floodPkSignal = registerSignal("floodPk");

EXECUTE_ON_STARTUP(
        cEnum * e = cEnum::find("inet::ChooseDestAddrMode");
        if (!e)
            enums.getInstance()->add(e = new cEnum("inet::ChooseDestAddrMode"));
        e->insert(UdpBasicFlooding::ONCE, "once");
        e->insert(UdpBasicFlooding::PER_BURST, "perBurst");
        e->insert(UdpBasicFlooding::PER_SEND, "perSend");
        );

UdpBasicFlooding::UdpBasicFlooding()
{
}

UdpBasicFlooding::~UdpBasicFlooding()
{
    cancelAndDelete(timerNext);
}

void UdpBasicFlooding::initialize(int stage)
{
    // because of AddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    ApplicationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        counter = 0;
        numSent = 0;
        numReceived = 0;
        numDeleted = 0;
        numDuplicated = 0;
        numFlood = 0;

        delayLimit = par("delayLimit");
        startTime = par("startTime");
        stopTime = par("stopTime");

        messageLengthPar = &par("messageLength");
        burstDurationPar = &par("burstDuration");
        sleepDurationPar = &par("sleepDuration");
        sendIntervalPar = &par("sendInterval");
        nextSleep = startTime;
        nextBurst = startTime;
        nextPkt = startTime;
        isSource = par("isSource");

        WATCH(numSent);
        WATCH(numReceived);
        WATCH(numDeleted);
        WATCH(numDuplicated);
        WATCH(numFlood);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER)
        processConfigure();
}

void UdpBasicFlooding::processConfigure()
{
    localPort = par("localPort");
    destPort = par("destPort");

    socket.setOutputGate(gate("socketOut"));
    socket.setCallback(this);
    socket.bind(localPort);
    socket.setBroadcast(true);

    outputInterfaceMulticastBroadcast.clear();
    if (strcmp(par("outputInterfaceMulticastBroadcast").stringValue(),"") != 0)
    {
        IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        const char *ports = par("outputInterfaceMulticastBroadcast");
        cStringTokenizer tokenizer(ports);
        const char *token;
        while ((token = tokenizer.nextToken()) != nullptr)
        {
            if (strstr(token, "ALL") != nullptr)
            {
                for (int i = 0; i < ift->getNumInterfaces(); i++)
                {
                    auto ie = ift->getInterface(i);
                    if (ie->isLoopback())
                        continue;
                    if (ie == nullptr)
                        throw cRuntimeError(this, "Invalid output interface name : %s", token);
                    outputInterfaceMulticastBroadcast.push_back(ie->getInterfaceId());
                }
            }
            else
            {
                auto ie = ift->findInterfaceByName(token);
                if (ie == nullptr)
                    throw cRuntimeError(this, "Invalid output interface name : %s", token);
                outputInterfaceMulticastBroadcast.push_back(ie->getInterfaceId());
            }
        }
    }
    myId = this->getParentModule()->getId();
}

void UdpBasicFlooding::processStart()
{
    nextSleep = simTime();
    nextBurst = simTime();
    nextPkt = simTime();
    activeBurst = false;
    if (addressModule && addressModule->getNumAddress() > 0) {
        if (chooseDestAddrMode == ONCE)
            destAddr = addressModule->choseNewModule();
        activeBurst = true;
    }
    timerNext->setKind(SEND);
    processSend();
}


Packet *UdpBasicFlooding::createPacket()
{
    if (!isSource)
        throw cRuntimeError("This module is not configured like sorce, cannot create packets");
    char msgName[32];
    sprintf(msgName, "UDPBasicAppData-%d", counter++);
    long msgByteLength = *messageLengthPar;
    Packet *pk = new Packet(msgName);
    const auto& payload = makeShared<ApplicationPacket>();
    payload->setChunkLength(B(msgByteLength));
    payload->setSequenceNumber(numSent);
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    pk->insertAtBack(payload);
    pk->addPar("sourceId") = getId();
    pk->addPar("msgId") = numSent;
    if (chooseDestAddrMode == ONCE)
        pk->addPar("destAddr") = destAddr;
    else
        pk->addPar("destAddr") = addressModule->choseNewModule();

    return pk;
}

void UdpBasicFlooding::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        switch (msg->getKind()) {
            case START:
                processStart();
                break;

            case SEND:
                processSend();
                break;

            case STOP:
                processStop();
                break;

            default:
                if (dynamic_cast<Packet*>(msg))
                {
                    // delayed packet
                    L3Address destAddr(Ipv4Address::ALLONES_ADDRESS);
                    sendBroadcast(destAddr, dynamic_cast<Packet*>(msg));
                }
                else
                    throw cRuntimeError("Invalid kind %d in self message", (int)msg->getKind());
        }
    }
    else
        socket.processMessage(msg);
}

void UdpBasicFlooding::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet
    processPacket(packet);
}

void UdpBasicFlooding::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void UdpBasicFlooding::socketClosed(UdpSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}


void UdpBasicFlooding::processPacket(Packet *pk)
{
    if (pk->getKind() == UDP_I_ERROR)
    {
        EV << "UDP error received\n";
        delete pk;
        return;
    }

    if (pk->hasPar("sourceId") && pk->hasPar("msgId"))
    {
        // duplicate control
        int moduleId = (int)pk->par("sourceId");
        int msgId = (int)pk->par("msgId");
        // check if this message has like origin this node
        if (moduleId == getId())
        {
            delete pk;
            return;
        }
        auto it = sourceSequence.find(moduleId);
        if (it != sourceSequence.end())
        {
            if (it->second >= msgId)
            {
                EV << "Out of order packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
                emit(outOfOrderPkSignal, pk);
                delete pk;
                numDuplicated++;
                return;
            }
            else
                it->second = msgId;
        }
        else
            sourceSequence[moduleId] = msgId;
    }

    if (delayLimit > 0)
    {
        if (simTime() - pk->getTimestamp() > delayLimit)
        {
            EV << "Old packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
            emit(dropPkSignal, pk);
            delete pk;
            numDeleted++;
            return;
        }
    }

    if (pk->hasPar("destAddr"))
    {
        int moduleId = (int)pk->par("destAddr");
        if (moduleId == myId)
        {
            EV << "Received packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
            emit(rcvdPkSignal, pk);
            numReceived++;
            delete pk;
            return;
        }
    }
    else
    {
        EV << "Received packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
        emit(rcvdPkSignal, pk);
        numReceived++;
    }

    EV_INFO << "Received packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
    emit(packetReceivedSignal, pk);

    auto l3Addresses = pk->getTag<L3AddressInd>();
    L3Address srcAddr = l3Addresses->getSrcAddress();
    L3Address destAddr = l3Addresses->getDestAddress();

    if (srcAddr.toIpv4() == Ipv4Address::ALLONES_ADDRESS && par("flooding").boolValue())
    {

        numFlood++;
        emit(floodPkSignal, pk);
        auto tag = pk->removeTag<CreationTimeTag>();
        pk->clearTags();
        *pk->addTag<CreationTimeTag>() = *tag;
        scheduleAt(simTime()+par("delay").doubleValue(),pk);
    }
    else
        delete pk;
}

void UdpBasicFlooding::generateBurst()
{
    simtime_t now = simTime();

    if (nextPkt < now)
        nextPkt = now;

    double sendInterval = sendIntervalPar->doubleValue();
    if (sendInterval <= 0.0)
        throw cRuntimeError("The sendInterval parameter must be bigger than 0");
    nextPkt += sendInterval;

    if (activeBurst && nextBurst <= now) // new burst
    {
        double burstDuration = burstDurationPar->doubleValue();
        if (burstDuration < 0.0)
            throw cRuntimeError("The burstDuration parameter mustn't be smaller than 0");
        double sleepDuration = sleepDurationPar->doubleValue();

        if (burstDuration == 0.0)
            activeBurst = false;
        else
        {
            if (sleepDuration < 0.0)
                throw cRuntimeError("The sleepDuration parameter mustn't be smaller than 0");
            nextSleep = now + burstDuration;
            nextBurst = nextSleep + sleepDuration;
        }
    }

    L3Address destAddr(Ipv4Address::ALLONES_ADDRESS);

    Packet *payload = createPacket();
    payload->setTimestamp();
    emit(sentPkSignal, payload);

    // Check address type
    sendBroadcast(destAddr, payload);

    numSent++;

    // Next timer
    if (activeBurst && nextPkt >= nextSleep)
        nextPkt = nextBurst;

    scheduleAt(nextPkt, timerNext);
}

void UdpBasicFlooding::finish()
{
    recordScalar("Total sent", numSent);
    recordScalar("Total received", numReceived);
    recordScalar("Total deleted", numDeleted);
}

bool UdpBasicFlooding::sendBroadcast(const L3Address &dest, Packet *pkt)
{
    if (!outputInterfaceMulticastBroadcast.empty() && (dest.isMulticast() || (dest.getType() != L3Address::IPv6 && dest.toIpv4() == Ipv4Address::ALLONES_ADDRESS)))
    {
        for (unsigned int i = 0; i < outputInterfaceMulticastBroadcast.size(); i++)
        {
            if (outputInterfaceMulticastBroadcast.size() - i > 1)
                socket.sendTo(pkt->dup(), dest, destPort);
            else
                socket.sendTo(pkt, dest, destPort);
        }
        return true;
    }
    return false;
}


void UdpBasicFlooding::handleStartOperation(LifecycleOperation *operation)
{
    simtime_t start = std::max(startTime, simTime());
    if (isSource)
    {
        if (addressModule == nullptr)
            addressModule = new AddressModule();
        addressModule->initModule(true);

        activeBurst = true;
        if (timerNext == nullptr)
              timerNext = new cMessage("UdpBasicFloodingTimer");
        if ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime)) {
            timerNext->setKind(START);
            scheduleAt(start, timerNext);
        }
    }
}

void UdpBasicFlooding::handleStopOperation(LifecycleOperation *operation)
{
    if (timerNext)
        cancelEvent(timerNext);
    activeBurst = false;
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void UdpBasicFlooding::handleCrashOperation(LifecycleOperation *operation)
{
    if (timerNext)
        cancelEvent(timerNext);
    activeBurst = false;
    if (operation->getRootModule() != getContainingNode(this)) // closes socket when the application crashed only
        socket.destroy(); // TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
}

}
