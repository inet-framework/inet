/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "inet/applications/ethernet/EtherTrafGen.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
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
    if (stage == INITSTAGE_APPLICATION_LAYER && isGenerator())
        timerMsg = new cMessage("generateNextPacket");

    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
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
            llcSocket.open(-1, ssap);
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
    llcSocket.close();     //TODO return false and waiting socket close
}

void EtherTrafGen::handleCrashOperation(LifecycleOperation *operation)
{
    cancelNextPacket();
    if (operation->getRootModule() != getContainingNode(this))     // closes socket when the application crashed only
        llcSocket.destroy();         //TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
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
    if (destAddress[0]) {
        if (!destMacAddress.tryParse(destAddress))
            destMacAddress = L3AddressResolver().resolve(destAddress, L3AddressResolver::ADDR_MAC).toMac();
    }
    return destMacAddress;
}

void EtherTrafGen::sendBurstPackets()
{
    int n = *numPacketsPerBurst;
    for (int i = 0; i < n; i++) {
        seqNum++;

        char msgname[40];
        sprintf(msgname, "pk-%d-%ld", getId(), seqNum);

        Packet *datapacket = new Packet(msgname, IEEE802CTRL_DATA);
        long len = *packetLength;
        const auto& payload = makeShared<ByteCountChunk>(B(len));
        payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
        datapacket->insertAtBack(payload);
        datapacket->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ieee8022);
        datapacket->addTag<MacAddressReq>()->setDestAddress(destMacAddress);
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

