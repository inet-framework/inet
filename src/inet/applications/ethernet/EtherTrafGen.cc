//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/applications/ethernet/EtherTrafGen.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(EtherTrafGen);

EtherTrafGen::EtherTrafGen()
{
    sendInterval = nullptr;
    numPacketsPerBurst = nullptr;
    packetLength = nullptr;
    timerMsg = nullptr;
}

EtherTrafGen::~EtherTrafGen()
{
    cancelAndDelete(timerMsg);
}

void EtherTrafGen::initialize(int stage)
{
    if (stage == INITSTAGE_APPLICATION_LAYER && isGenerator()) {
        timerMsg = new cMessage("generateNextPacket");
        outInterface = CHK(interfaceTable->findInterfaceByName(par("interface")))->getInterfaceId();
    }

    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        interfaceTable.reference(this, "interfaceTableModule", true);
        sendInterval = &par("sendInterval");
        numPacketsPerBurst = &par("numPacketsPerBurst");
        packetLength = &par("packetLength");
        ssap = par("ssap");
        dsap = par("dsap");

        seqNum = 0;
        WATCH(seqNum);

        // statistics
        packetsSent = packetsReceived = 0;
        WATCH(packetsSent);
        WATCH(packetsReceived);

        startTime = par("startTime");
        stopTime = par("stopTime");
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        llcSocket.setOutputGate(gate("out"));
    }
}

void EtherTrafGen::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        if (msg->getKind() == START) {
            llcSocket.open(-1, ssap, -1);
            destMacAddress = resolveDestMacAddress();
            // if no dest address given, nothing to do
            if (destMacAddress.isUnspecified())
                return;
        }
        sendBurstPackets();
        scheduleNextPacket(simTime());
    }
    else
        receivePacket(check_and_cast<Packet *>(msg));
}

void EtherTrafGen::handleStartOperation(LifecycleOperation *operation)
{
    if (isGenerator()) {
        scheduleNextPacket(-1);
    }
}

void EtherTrafGen::handleStopOperation(LifecycleOperation *operation)
{
    cancelNextPacket();
    llcSocket.close(); // TODO return false and waiting socket close
}

void EtherTrafGen::handleCrashOperation(LifecycleOperation *operation)
{
    cancelNextPacket();
    if (operation->getRootModule() != getContainingNode(this)) // closes socket when the application crashed only
        llcSocket.destroy(); // TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
}

bool EtherTrafGen::isGenerator()
{
    return par("destAddress").stringValue()[0];
}

void EtherTrafGen::scheduleNextPacket(simtime_t previous)
{
    simtime_t next;
    if (previous == -1) {
        next = simTime() <= startTime ? startTime : simTime();
        timerMsg->setKind(START);
    }
    else {
        next = previous + *sendInterval;
        timerMsg->setKind(NEXT);
    }
    if (stopTime < SIMTIME_ZERO || next < stopTime)
        scheduleAt(next, timerMsg);
}

void EtherTrafGen::cancelNextPacket()
{
    cancelEvent(timerMsg);
}

MacAddress EtherTrafGen::resolveDestMacAddress()
{
    MacAddress destMacAddress;
    const char *destAddress = par("destAddress");
    if (destAddress[0])
        destMacAddress = L3AddressResolver().resolve(destAddress, L3AddressResolver::ADDR_MAC).toMac();
    return destMacAddress;
}

void EtherTrafGen::sendBurstPackets()
{
    int n = *numPacketsPerBurst;
    for (int i = 0; i < n; i++) {
        seqNum++;

        char msgname[40];
        sprintf(msgname, "pk-%d-%ld", getId(), seqNum);

        Packet *datapacket = new Packet(msgname, SOCKET_C_DATA);
        long len = *packetLength;
        const auto& payload = makeShared<ByteCountChunk>(B(len));
        payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
        datapacket->insertAtBack(payload);
        datapacket->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ieee8022llc);
        datapacket->addTag<MacAddressReq>()->setDestAddress(destMacAddress);
        datapacket->addTag<InterfaceReq>()->setInterfaceId(outInterface);
        auto sapTag = datapacket->addTag<Ieee802SapReq>();
        sapTag->setSsap(ssap);
        sapTag->setDsap(dsap);

        EV_INFO << "Send packet `" << msgname << "' dest=" << destMacAddress << " length=" << len << "B ssap/dsap=" << ssap << "/" << dsap << "\n";
        emit(packetSentSignal, datapacket);
        send(datapacket, "out");
        packetsSent++;
    }
}

void EtherTrafGen::receivePacket(Packet *msg)
{
    EV_INFO << "Received packet `" << msg->getName() << "' length= " << msg->getByteLength() << "B\n";

    packetsReceived++;
    emit(packetReceivedSignal, msg);
    delete msg;
}

void EtherTrafGen::finish()
{
    cancelAndDelete(timerMsg);
    timerMsg = nullptr;
}

} // namespace inet

