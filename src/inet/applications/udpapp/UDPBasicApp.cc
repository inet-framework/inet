//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2011 Andras Varga
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

#include "inet/applications/udpapp/UDPBasicApp.h"

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/transportlayer/contract/udp/UDPControlInfo_m.h"

namespace inet {

Define_Module(UDPBasicApp);

simsignal_t UDPBasicApp::sentPkSignal = registerSignal("sentPk");
simsignal_t UDPBasicApp::rcvdPkSignal = registerSignal("rcvdPk");


UDPBasicApp::~UDPBasicApp()
{
    cancelAndDelete(selfMsg);
}

void UDPBasicApp::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        numSent = 0;
        numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);

        localPort = par("localPort");
        destPort = par("destPort");
        startTime = par("startTime").doubleValue();
        stopTime = par("stopTime").doubleValue();
        packetName = par("packetName");
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        selfMsg = new cMessage("sendTimer");

        sendIntervalPar = &par("sendInterval");
        bool sendIntervalGiven = sendIntervalPar->isExpression() || sendIntervalPar->doubleValue() > 0; // not a constant zero or negative number
        const char *script = par("sendScript");
        parseScript(script);
        if (sendIntervalGiven && !sendScript.empty())
            throw cRuntimeError("Cannot use both sendScript and sendInterval");
        useSendScript = !sendIntervalGiven; // note: instead of !sendScript.empty(), because sendInterval=0 should result in sending turned off
    }
}

void UDPBasicApp::finish()
{
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    ApplicationBase::finish();
}

void UDPBasicApp::setSocketOptions()
{
    int timeToLive = par("timeToLive");
    if (timeToLive != -1)
        socket.setTimeToLive(timeToLive);

    int typeOfService = par("typeOfService");
    if (typeOfService != -1)
        socket.setTypeOfService(typeOfService);

    const char *multicastInterface = par("multicastInterface");
    if (multicastInterface[0]) {
        IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        InterfaceEntry *ie = ift->getInterfaceByName(multicastInterface);
        if (!ie)
            throw cRuntimeError("Wrong multicastInterface setting: no interface named \"%s\"", multicastInterface);
        socket.setMulticastOutputInterface(ie->getInterfaceId());
    }

    bool receiveBroadcast = par("receiveBroadcast");
    if (receiveBroadcast)
        socket.setBroadcast(true);

    bool joinLocalMulticastGroups = par("joinLocalMulticastGroups");
    if (joinLocalMulticastGroups) {
        MulticastGroupList mgl = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)->collectMulticastGroups();
        socket.joinLocalMulticastGroups(mgl);
    }
}

void UDPBasicApp::parseScript(const char *script)
{
    const char *s = script;

    EV_DEBUG << "parse script \"" << script << "\"\n";
    while (*s) {
        // parse time
        while (isspace(*s))
            s++;

        if (!*s || *s == ';')
            break;

        const char *s0 = s;
        simtime_t tSend = strtod(s, &const_cast<char *&>(s));

        if (s == s0)
            throw cRuntimeError("Syntax error in script: simulation time expected");

        // parse number of bytes
        while (isspace(*s))
            s++;

        if (!isdigit(*s))
            throw cRuntimeError("Syntax error in script: number of bytes expected");

        long numBytes = strtol(s, nullptr, 10);

        while (isdigit(*s))
            s++;

        // add command
        EV_DEBUG << " add command (" << tSend << "s, " << "B)\n";
        sendScript.push_back(SendScriptItem(tSend, numBytes));

        // skip delimiter
        while (isspace(*s))
            s++;

        if (!*s)
            break;

        if (*s != ';')
            throw cRuntimeError("Syntax error in script: separator ';' missing");

        s++;

        while (isspace(*s))
            s++;
    }
    EV_DEBUG << "parser finished\n";
}

L3Address UDPBasicApp::chooseDestAddr()
{
    int k = intrand(destAddresses.size());
    if (destAddresses[k].isLinkLocal()) {    // KLUDGE for IPv6
        const char *destAddrs = par("destAddresses");
        cStringTokenizer tokenizer(destAddrs);
        const char *token = nullptr;

        for (int i = 0; i <= k; ++i)
            token = tokenizer.nextToken();
        destAddresses[k] = L3AddressResolver().resolve(token);
    }
    return destAddresses[k];
}

void UDPBasicApp::sendPacket(long bytes)
{
    std::ostringstream str;
    str << packetName << "-" << numSent;
    ApplicationPacket *payload = new ApplicationPacket(str.str().c_str());
    payload->setByteLength(bytes);
    payload->setSequenceNumber(numSent);

    L3Address destAddr = chooseDestAddr();

    emit(sentPkSignal, payload);
    socket.sendTo(payload, destAddr, destPort);
    numSent++;
}

void UDPBasicApp::processStart()
{
    socket.setOutputGate(gate("udpOut"));
    const char *localAddress = par("localAddress");
    socket.bind(*localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);
    setSocketOptions();

    const char *destAddrs = par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;

    while ((token = tokenizer.nextToken()) != nullptr) {
        L3Address result;
        L3AddressResolver().tryResolve(token, result);
        if (result.isUnspecified())
            EV_ERROR << "cannot resolve destination address: " << token << endl;
        else
            destAddresses.push_back(result);
    }

    if (!destAddresses.empty()) {
        if (!useSendScript)
            processSend();
        else {
            // find first script entry whose time is >= simTime()
            for (sendScriptIndex = 0; sendScriptIndex < sendScript.size() && sendScript[sendScriptIndex].tSend < simTime(); sendScriptIndex++)
                /*no-op*/;
            scheduleNextSending();
        }
    }
    else {
        if (stopTime >= SIMTIME_ZERO) {
            selfMsg->setKind(STOP);
            scheduleAt(stopTime, selfMsg);
        }
    }
}

void UDPBasicApp::processSend()
{
    long bytes = useSendScript ? sendScript[sendScriptIndex++].numBytes : (long)par("messageLength").intValue();
    sendPacket(bytes);

    scheduleNextSending();
}

void UDPBasicApp::scheduleNextSending()
{
    simtime_t nextSendTime;
    if (!useSendScript)
        nextSendTime = simTime() + sendIntervalPar->doubleValue();
    else
        nextSendTime = sendScriptIndex < sendScript.size() ? sendScript[sendScriptIndex].tSend : -1;

    if (nextSendTime < SIMTIME_ZERO && stopTime < SIMTIME_ZERO) {  // neither is valid
        // nothing to do
    }
    else if (stopTime < SIMTIME_ZERO) { // only nextSendTime is valid
        selfMsg->setKind(SEND);
        scheduleAt(nextSendTime, selfMsg);
    }
    else if (nextSendTime < SIMTIME_ZERO) {  // only stopTime is valid
        selfMsg->setKind(STOP);
        scheduleAt(stopTime, selfMsg);
    }
    else { // both are valid
        if (nextSendTime < stopTime) {
            selfMsg->setKind(SEND);
            scheduleAt(nextSendTime, selfMsg);
        }
        else {
            selfMsg->setKind(STOP);
            scheduleAt(stopTime, selfMsg);
        }
    }
}

void UDPBasicApp::processStop()
{
    socket.close();
}

void UDPBasicApp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        ASSERT(msg == selfMsg);
        switch (selfMsg->getKind()) {
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
                throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
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
}

void UDPBasicApp::refreshDisplay() const
{
    char buf[100];
    sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
    getDisplayString().setTagArg("t", 0, buf);
}

void UDPBasicApp::processPacket(cPacket *pk)
{
    emit(rcvdPkSignal, pk);
    EV_INFO << "Received packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
    delete pk;
    numReceived++;
}

bool UDPBasicApp::handleNodeStart(IDoneCallback *doneCallback)
{
    simtime_t start = std::max(startTime, simTime());
    if ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime)) {
        selfMsg->setKind(START);
        scheduleAt(start, selfMsg);
    }
    return true;
}

bool UDPBasicApp::handleNodeShutdown(IDoneCallback *doneCallback)
{
    if (selfMsg)
        cancelEvent(selfMsg);
    //TODO if(socket.isOpened()) socket.close();
    return true;
}

void UDPBasicApp::handleNodeCrash()
{
    if (selfMsg)
        cancelEvent(selfMsg);
}

} // namespace inet

