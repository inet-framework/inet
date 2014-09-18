//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2007 Universidad de Málaga
// Copyright (C) 2011 Zoltan Bojthe
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

#include "inet/applications/udpapp/UDPBasicBurst.h"

#include "inet/transportlayer/contract/udp/UDPControlInfo_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

EXECUTE_ON_STARTUP(
        cEnum * e = cEnum::find("inet::ChooseDestAddrMode");
        if (!e)
            enums.getInstance()->add(e = new cEnum("inet::ChooseDestAddrMode"));
        e->insert(UDPBasicBurst::ONCE, "once");
        e->insert(UDPBasicBurst::PER_BURST, "perBurst");
        e->insert(UDPBasicBurst::PER_SEND, "perSend");
        );

Define_Module(UDPBasicBurst);

int UDPBasicBurst::counter;

simsignal_t UDPBasicBurst::sentPkSignal = registerSignal("sentPk");
simsignal_t UDPBasicBurst::rcvdPkSignal = registerSignal("rcvdPk");
simsignal_t UDPBasicBurst::outOfOrderPkSignal = registerSignal("outOfOrderPk");
simsignal_t UDPBasicBurst::dropPkSignal = registerSignal("dropPk");

UDPBasicBurst::UDPBasicBurst()
{
    messageLengthPar = NULL;
    burstDurationPar = NULL;
    sleepDurationPar = NULL;
    sendIntervalPar = NULL;
    timerNext = NULL;
}

UDPBasicBurst::~UDPBasicBurst()
{
    cancelAndDelete(timerNext);
}

void UDPBasicBurst::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        counter = 0;
        numSent = 0;
        numReceived = 0;
        numDeleted = 0;
        numDuplicated = 0;

        delayLimit = par("delayLimit");
        startTime = par("startTime");
        stopTime = par("stopTime");
        if (stopTime >= SIMTIME_ZERO && stopTime <= startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");

        messageLengthPar = &par("messageLength");
        burstDurationPar = &par("burstDuration");
        sleepDurationPar = &par("sleepDuration");
        sendIntervalPar = &par("sendInterval");
        nextSleep = startTime;
        nextBurst = startTime;
        nextPkt = startTime;

        destAddrRNG = par("destAddrRNG");
        const char *addrModeStr = par("chooseDestAddrMode").stringValue();
        int addrMode = cEnum::get("inet::ChooseDestAddrMode")->lookup(addrModeStr);
        if (addrMode == -1)
            throw cRuntimeError("Invalid chooseDestAddrMode: '%s'", addrModeStr);
        chooseDestAddrMode = (ChooseDestAddrMode)addrMode;

        WATCH(numSent);
        WATCH(numReceived);
        WATCH(numDeleted);
        WATCH(numDuplicated);

        localPort = par("localPort");
        destPort = par("destPort");

        timerNext = new cMessage("UDPBasicBurstTimer");
    }
}

L3Address UDPBasicBurst::chooseDestAddr()
{
    if (destAddresses.size() == 1)
        return destAddresses[0];

    int k = genk_intrand(destAddrRNG, destAddresses.size());
    return destAddresses[k];
}

cPacket *UDPBasicBurst::createPacket()
{
    char msgName[32];
    sprintf(msgName, "UDPBasicAppData-%d", counter++);
    long msgByteLength = messageLengthPar->longValue();
    cPacket *payload = new cPacket(msgName);
    payload->setByteLength(msgByteLength);
    payload->addPar("sourceId") = getId();
    payload->addPar("msgId") = numSent;

    return payload;
}

void UDPBasicBurst::processStart()
{
    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);

    const char *destAddrs = par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;
    bool excludeLocalDestAddresses = par("excludeLocalDestAddresses").boolValue();

    IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);

    while ((token = tokenizer.nextToken()) != NULL) {
        if (strstr(token, "Broadcast") != NULL)
            destAddresses.push_back(IPv4Address::ALLONES_ADDRESS);
        else {
            L3Address addr = L3AddressResolver().resolve(token);
            if (excludeLocalDestAddresses && ift && ift->isLocalAddress(addr))
                continue;
            destAddresses.push_back(addr);
        }
    }

    nextSleep = simTime();
    nextBurst = simTime();
    nextPkt = simTime();
    activeBurst = false;

    isSource = !destAddresses.empty();

    if (isSource) {
        if (chooseDestAddrMode == ONCE)
            destAddr = chooseDestAddr();

        activeBurst = true;
    }
    timerNext->setKind(SEND);
    processSend();
}

void UDPBasicBurst::processSend()
{
    if (stopTime < SIMTIME_ZERO || simTime() < stopTime) {
        // send and reschedule next sending
        if (isSource) // if the node is a sink, don't generate messages
            generateBurst();
    }
}

void UDPBasicBurst::processStop()
{
    socket.close();
}

void UDPBasicBurst::handleMessageWhenUp(cMessage *msg)
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
                throw cRuntimeError("Invalid kind %d in self message", (int)msg->getKind());
        }
    }
    else if (msg->getKind() == UDP_I_DATA) {
        // process incoming packet
        processPacket(PK(msg));
    }
    else if (msg->getKind() == UDP_I_ERROR) {
        EV_WARN << "Ignoring UDP error report\n";
        delete msg;
    }
    else {
        throw cRuntimeError("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }

    if (ev.isGUI()) {
        char buf[40];
        sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void UDPBasicBurst::processPacket(cPacket *pk)
{
    if (pk->getKind() == UDP_I_ERROR) {
        EV_WARN << "UDP error received\n";
        delete pk;
        return;
    }

    if (pk->hasPar("sourceId") && pk->hasPar("msgId")) {
        // duplicate control
        int moduleId = (int)pk->par("sourceId");
        int msgId = (int)pk->par("msgId");
        SourceSequence::iterator it = sourceSequence.find(moduleId);
        if (it != sourceSequence.end()) {
            if (it->second >= msgId) {
                EV_DEBUG << "Out of order packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
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

    if (delayLimit > 0) {
        if (simTime() - pk->getTimestamp() > delayLimit) {
            EV_DEBUG << "Old packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
            emit(dropPkSignal, pk);
            delete pk;
            numDeleted++;
            return;
        }
    }

    EV_INFO << "Received packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
    emit(rcvdPkSignal, pk);
    numReceived++;
    delete pk;
}

void UDPBasicBurst::generateBurst()
{
    simtime_t now = simTime();

    if (nextPkt < now)
        nextPkt = now;

    double sendInterval = sendIntervalPar->doubleValue();
    if (sendInterval <= 0.0)
        throw cRuntimeError("The sendInterval parameter must be bigger than 0");
    nextPkt += sendInterval;

    if (activeBurst && nextBurst <= now) {    // new burst
        double burstDuration = burstDurationPar->doubleValue();
        if (burstDuration < 0.0)
            throw cRuntimeError("The burstDuration parameter mustn't be smaller than 0");
        double sleepDuration = sleepDurationPar->doubleValue();

        if (burstDuration == 0.0)
            activeBurst = false;
        else {
            if (sleepDuration < 0.0)
                throw cRuntimeError("The sleepDuration parameter mustn't be smaller than 0");
            nextSleep = now + burstDuration;
            nextBurst = nextSleep + sleepDuration;
        }

        if (chooseDestAddrMode == PER_BURST)
            destAddr = chooseDestAddr();
    }

    if (chooseDestAddrMode == PER_SEND)
        destAddr = chooseDestAddr();

    cPacket *payload = createPacket();
    payload->setTimestamp();
    emit(sentPkSignal, payload);
    socket.sendTo(payload, destAddr, destPort);
    numSent++;

    // Next timer
    if (activeBurst && nextPkt >= nextSleep)
        nextPkt = nextBurst;

    if (stopTime >= SIMTIME_ZERO && nextPkt >= stopTime) {
        timerNext->setKind(STOP);
        nextPkt = stopTime;
    }
    scheduleAt(nextPkt, timerNext);
}

void UDPBasicBurst::finish()
{
    recordScalar("Total sent", numSent);
    recordScalar("Total received", numReceived);
    recordScalar("Total deleted", numDeleted);
    ApplicationBase::finish();
}

bool UDPBasicBurst::handleNodeStart(IDoneCallback *doneCallback)
{
    simtime_t start = std::max(startTime, simTime());

    if ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime)) {
        timerNext->setKind(START);
        scheduleAt(start, timerNext);
    }

    return true;
}

bool UDPBasicBurst::handleNodeShutdown(IDoneCallback *doneCallback)
{
    if (timerNext)
        cancelEvent(timerNext);
    activeBurst = false;
    //TODO if(socket.isOpened()) socket.close();
    return true;
}

void UDPBasicBurst::handleNodeCrash()
{
    if (timerNext)
        cancelEvent(timerNext);
    activeBurst = false;
}

} // namespace inet

