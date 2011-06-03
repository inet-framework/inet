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


#include "UDPBasicBurst.h"

#include "UDPControlInfo_m.h"
#include "IPvXAddressResolver.h"


EXECUTE_ON_STARTUP(
    cEnum *e = cEnum::find("ChooseDestAddrMode");
    if (!e) enums.getInstance()->add(e = new cEnum("ChooseDestAddrMode"));
    e->insert(UDPBasicBurst::ONCE, "once");
    e->insert(UDPBasicBurst::PER_BURST, "perBurst");
    e->insert(UDPBasicBurst::PER_SEND, "perSend");
);

Define_Module(UDPBasicBurst);

int UDPBasicBurst::counter;

simsignal_t UDPBasicBurst::sentPkSignal = SIMSIGNAL_NULL;
simsignal_t UDPBasicBurst::rcvdPkSignal = SIMSIGNAL_NULL;
simsignal_t UDPBasicBurst::duplPkSignal = SIMSIGNAL_NULL;
simsignal_t UDPBasicBurst::dropPkSignal = SIMSIGNAL_NULL;
simsignal_t UDPBasicBurst::endToEndDelaySignal = SIMSIGNAL_NULL;

UDPBasicBurst::UDPBasicBurst()
{
    messageLengthPar = NULL;
    burstDurationPar = NULL;
    sleepDurationPar = NULL;
    messageFreqPar = NULL;
    timerNext = NULL;
}

UDPBasicBurst::~UDPBasicBurst()
{
    cancelAndDelete(timerNext);
}

void UDPBasicBurst::initialize(int stage)
{
    // because of IPvXAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage != 3)
        return;

    counter = 0;
    numSent = 0;
    numReceived = 0;
    numDeleted = 0;
    numDuplicated = 0;

    delayLimit = par("delayLimit");
    simtime_t startTime = par("startTime");
    stopTime = par("stopTime");

    messageLengthPar = &par("messageLength");
    burstDurationPar = &par("burstDuration");
    sleepDurationPar = &par("sleepDuration");
    messageFreqPar = &par("messageFreq");
    nextSleep = startTime;
    nextBurst = startTime;
    nextPkt = startTime;

    destAddrRNG = par("destAddrRNG");
    const char *addrModeStr = par("chooseDestAddrMode").stringValue();
    int addrMode = cEnum::get("ChooseDestAddrMode")->lookup(addrModeStr);
    if (addrMode == -1)
        throw cRuntimeError(this, "Invalid chooseDestAddrMode: '%s'", addrModeStr);
    chooseDestAddrMode = (ChooseDestAddrMode)addrMode;

    WATCH(numSent);
    WATCH(numReceived);
    WATCH(numDeleted);
    WATCH(numDuplicated);

    localPort = par("localPort");
    destPort = par("destPort");

    if (localPort != -1)
        bindToPort(localPort);

    const char *destAddrs = par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;
    const char * random_add;

    isSink = false;

    IPvXAddress myAddr = IPvXAddressResolver().resolve(this->getParentModule()->getFullPath().c_str());
    while ((token = tokenizer.nextToken()) != NULL)
    {
        if (strstr(token, "Broadcast") != NULL)
            destAddresses.push_back(IPv4Address::ALLONES_ADDRESS);
        else
        {
            IPvXAddress addr = IPvXAddressResolver().resolve(token);
            if (addr != myAddr)
                destAddresses.push_back(addr);
        }
    }

    if (destAddresses.empty())
    {
        isSink = true;
        return;
    }

    destAddr = IPvXAddress(); // clear destAddr

    if (chooseDestAddrMode == ONCE)
        destAddr = chooseDestAddr();

    timerNext = new cMessage("UDPBasicBurstTimer");
    scheduleAt(startTime, timerNext);

    sentPkSignal = registerSignal("sentPk");
    rcvdPkSignal = registerSignal("rcvdPk");
    duplPkSignal = registerSignal("duplPk");
    dropPkSignal = registerSignal("dropPk");
    endToEndDelaySignal = registerSignal("endToEndDelay");
}

IPvXAddress UDPBasicBurst::chooseDestAddr()
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

void UDPBasicBurst::sendPacket()
{
    cPacket *payload = createPacket();
    emit(sentPkSignal, payload);
    sendToUDP(payload, localPort, destAddr, destPort);
    numSent++;
}

void UDPBasicBurst::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        if (stopTime <= 0 || simTime() < stopTime)
        {
            // send and reschedule next sending
            if (!isSink) // if the node is a sink, don't generate messages
                generateBurst();
        }
    }
    else
    {
        // process incoming packet
        processPacket(PK(msg));
    }

    if (ev.isGUI())
    {
        char buf[40];
        sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void UDPBasicBurst::processPacket(cPacket *msg)
{
    if (msg->getKind() == UDP_I_ERROR)
    {
        delete msg;
        return;
    }

    if (msg->hasPar("sourceId"))
    {
        // duplicate control
        int moduleId = (int)msg->par("sourceId");
        int msgId = (int)msg->par("msgId");
        SourceSequence::iterator it = sourceSequence.find(moduleId);
        if (it != sourceSequence.end())
        {
            if (it->second >= msgId)
            {
                EV << "Duplicated packet: ";
                printPacket(msg);
                emit(duplPkSignal, msg);
                delete msg;
                numDuplicated++;
                return;
            }
            else
                it->second = msgId;
        }
        else
            sourceSequence[moduleId] = msgId;
    }

    if (delayLimit >= 0)
    {
        if (simTime() - msg->getTimestamp() > delayLimit)
        {
            EV << "Old packet: ";
            printPacket(msg);
            emit(dropPkSignal, msg);
            delete msg;
            numDeleted++;
            return;
        }
    }

    EV << "Received packet: ";
    printPacket(msg);
    emit(endToEndDelaySignal, simTime() - msg->getTimestamp());
    emit(rcvdPkSignal, msg);
    numReceived++;
    delete msg;
}

void UDPBasicBurst::generateBurst()
{
    simtime_t now = simTime();

    if (nextPkt < now)
        nextPkt = now;

    nextPkt += messageFreqPar->doubleValue();

    if (nextBurst <= now) // new burst
    {
        nextSleep = now + burstDurationPar->doubleValue();
        nextBurst = nextSleep + sleepDurationPar->doubleValue();
        if (nextBurst <= now || nextBurst <= nextPkt)
            nextBurst = nextPkt;

        if (chooseDestAddrMode == PER_BURST)
            destAddr = chooseDestAddr();
    }

    if (chooseDestAddrMode == PER_SEND)
        destAddr = chooseDestAddr();

    cPacket *payload = createPacket();
    payload->setTimestamp();
    emit(sentPkSignal, payload);
    sendToUDP(payload, localPort, destAddr, destPort);
    numSent++;

    // Next timer
    if (nextPkt >= nextSleep)
        nextPkt = nextBurst;

    scheduleAt(nextPkt, timerNext);
}

void UDPBasicBurst::finish()
{
    recordScalar("Total sent", numSent);
    recordScalar("Total received", numReceived);
    recordScalar("Total deleted", numDeleted);
}

