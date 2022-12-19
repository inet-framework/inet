//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004-2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/applications/generic/IpvxTrafGen.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IL3AddressType.h"

namespace inet {

Define_Module(IpvxTrafGen);

IpvxTrafGen::IpvxTrafGen()
{
}

IpvxTrafGen::~IpvxTrafGen()
{
    cancelAndDelete(timer);
}

void IpvxTrafGen::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        int protocolId = par("protocol");
        if (protocolId < 143 || protocolId > 254)
            throw cRuntimeError("invalid protocol id %d, accepts only between 143 and 254", protocolId);
        protocol = ProtocolGroup::getIpProtocolGroup()->findProtocol(protocolId);
        if (!protocol) {
            std::string name = "prot_" + std::to_string(protocolId);
            protocol = new Protocol(name.c_str(), name.c_str());
            ProtocolGroup::getIpProtocolGroup()->addProtocol(protocolId, protocol);
        }
        numPackets = par("numPackets");
        startTime = par("startTime");
        stopTime = par("stopTime");
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");

        packetLengthPar = &par("packetLength");
        sendIntervalPar = &par("sendInterval");

        timer = new cMessage("sendTimer");

        numSent = 0;
        numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        registerProtocol(*protocol, gate("ipOut"), gate("ipIn"));
    }
}

void IpvxTrafGen::startApp()
{
    if (isEnabled())
        scheduleNextPacket(-1);
}

void IpvxTrafGen::handleMessageWhenUp(cMessage *msg)
{
    if (msg == timer) {
        if (msg->getKind() == START) {
            destAddresses.clear();
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
        }
        if (!destAddresses.empty()) {
            sendPacket();
            if (isEnabled())
                scheduleNextPacket(simTime());
        }
    }
    else
        processPacket(check_and_cast<Packet *>(msg));
}

void IpvxTrafGen::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();

    char buf[40];
    sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
    getDisplayString().setTagArg("t", 0, buf);
}

void IpvxTrafGen::scheduleNextPacket(simtime_t previous)
{
    simtime_t next;
    if (previous == -1) {
        next = simTime() <= startTime ? startTime : simTime();
        timer->setKind(START);
    }
    else {
        next = previous + *sendIntervalPar;
        timer->setKind(NEXT);
    }
    if (stopTime < SIMTIME_ZERO || next < stopTime)
        scheduleAt(next, timer);
}

void IpvxTrafGen::cancelNextPacket()
{
    cancelEvent(timer);
}

bool IpvxTrafGen::isEnabled()
{
    return numPackets == -1 || numSent < numPackets;
}

L3Address IpvxTrafGen::chooseDestAddr()
{
    int k = intrand(destAddresses.size());
    return destAddresses[k];
}

void IpvxTrafGen::sendPacket()
{
    char msgName[32];
    sprintf(msgName, "appData-%d", numSent);

    Packet *packet = new Packet(msgName);
    const auto& payload = makeShared<ByteCountChunk>(B(*packetLengthPar));
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);

    L3Address destAddr = chooseDestAddr();

    const IL3AddressType *addressType = destAddr.getAddressType();
    packet->addTag<PacketProtocolTag>()->setProtocol(protocol);
    packet->addTag<DispatchProtocolReq>()->setProtocol(addressType->getNetworkProtocol());
    packet->addTag<L3AddressReq>()->setDestAddress(destAddr);

    EV_INFO << "Sending packet: ";
    printPacket(packet);
    emit(packetSentSignal, packet);
    send(packet, "ipOut");
    numSent++;
}

void IpvxTrafGen::printPacket(Packet *msg)
{
    L3Address src, dest;
    int protocol = -1;
    auto *ctrl = msg->getControlInfo();
    if (ctrl != nullptr) {
        protocol = ProtocolGroup::getIpProtocolGroup()->getProtocolNumber(msg->getTag<PacketProtocolTag>()->getProtocol());
    }
    Ptr<const L3AddressTagBase> addresses = msg->findTag<L3AddressReq>();
    if (addresses == nullptr)
        addresses = msg->findTag<L3AddressInd>();
    if (addresses != nullptr) {
        src = addresses->getSrcAddress();
        dest = addresses->getDestAddress();
    }

    EV_INFO << msg << endl;
    EV_INFO << "Payload length: " << msg->getByteLength() << " bytes" << endl;

    if (protocol != -1)
        EV_INFO << "src: " << src << "  dest: " << dest << "  protocol=" << protocol << "\n";
}

void IpvxTrafGen::processPacket(Packet *msg)
{
    emit(packetReceivedSignal, msg);
    EV_INFO << "Received packet: ";
    printPacket(msg);
    delete msg;
    numReceived++;
}

void IpvxTrafGen::handleStartOperation(LifecycleOperation *operation)
{
    startApp();
}

void IpvxTrafGen::handleStopOperation(LifecycleOperation *operation)
{
    cancelNextPacket();
}

void IpvxTrafGen::handleCrashOperation(LifecycleOperation *operation)
{
    cancelNextPacket();
}

} // namespace inet

