// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//


#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/StringFormat.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ethernet/EtherMacBase.h"
#include "inet/linklayer/ethernet/Ethernet.h"
#include "inet/linklayer/ethernet/switch/MacRelayUnit.h"

namespace inet {

Define_Module(MacRelayUnit);

void MacRelayUnit::initialize(int stage)
{
    LayeredProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        numProcessedFrames = numDiscardedFrames = 0;

        macTable = getModuleFromPar<IMacAddressTable>(par("macTableModule"), this);
        ifTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);

        WATCH(numProcessedFrames);
        WATCH(numDiscardedFrames);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        registerProtocol(Protocol::ethernetMac, gate("ifOut"), gate("ifIn"));
    }
}

void MacRelayUnit::handleLowerPacket(Packet *packet)
{
    handleAndDispatchFrame(packet);
    updateDisplayString();
}

void MacRelayUnit::updateDisplayString() const
{
    auto text = StringFormat::formatString(par("displayStringTextFormat"), [&] (char directive) {
        static std::string result;
        switch (directive) {
            case 'p':
                result = std::to_string(numProcessedFrames);
                break;
            case 'd':
                result = std::to_string(numDiscardedFrames);
                break;
            default:
                throw cRuntimeError("Unknown directive: %c", directive);
        }
        return result.c_str();
    });
    getDisplayString().setTagArg("t", 0, text);
}

void MacRelayUnit::broadcast(Packet *packet, int arrivalInterfaceId)
{
    EV_DETAIL << "Broadcast frame " << packet << endl;

    int numPorts = ifTable->getNumInterfaces();
    for (int i = 0; i < numPorts; i++) {
        InterfaceEntry *ie = ifTable->getInterface(i);
        if (ie->isLoopback() || !ie->isBroadcast())
            continue;
        int ifId = ie->getInterfaceId();
        if (arrivalInterfaceId != ifId) {
            Packet *dupFrame = packet->dup();
            dupFrame->addTagIfAbsent<InterfaceReq>()->setInterfaceId(ifId);
            emit(packetSentToLowerSignal, dupFrame);
            send(dupFrame, "ifOut");
        }
    }
    delete packet;
}

void MacRelayUnit::handleAndDispatchFrame(Packet *packet)
{
    //FIXME : should handle multicast mac addresses correctly

    auto macAddressInd = packet->getTag<MacAddressInd>();
    int arrivalInterfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();

    auto& oldPacketProtocolTag = packet->removeTag<PacketProtocolTag>();
    packet->clearTags();
    auto newPacketProtocolTag = packet->addTag<PacketProtocolTag>();
    *newPacketProtocolTag = *oldPacketProtocolTag;
    auto& macAddressReq = packet->addTag<MacAddressReq>();
    macAddressReq->setSrcAddress(macAddressInd->getSrcAddress());
    macAddressReq->setDestAddress(macAddressInd->getDestAddress());
    packet->trim();

    numProcessedFrames++;

    // update address table
    learn(macAddressInd->getSrcAddress(), arrivalInterfaceId);

    // handle broadcast frames first
    if (macAddressInd->getDestAddress().isBroadcast()) {
        EV << "Broadcasting broadcast frame " << packet->getName() << endl;
        broadcast(packet, arrivalInterfaceId);
        return;
    }

    // Finds output port of destination address and sends to output port
    // if not found then broadcasts to all other ports instead
    int outputInterfaceId = macTable->getInterfaceIdForAddress(macAddressInd->getDestAddress());
    // should not send out the same frame on the same ethernet port
    // (although wireless ports are ok to receive the same message)
    if (arrivalInterfaceId == outputInterfaceId) {
        EV << "Output port is same as input port, " << packet->getFullName()
           << " dest " << macAddressInd->getDestAddress() << ", discarding frame\n";
        numDiscardedFrames++;
        delete packet;
        return;
    }

    if (outputInterfaceId >= 0) {
        EV << "Sending frame " << packet->getName() << " with dest address " << macAddressInd->getDestAddress() << " to port " << outputInterfaceId << endl;
        packet->addTag<InterfaceReq>()->setInterfaceId(outputInterfaceId);
        emit(packetSentToLowerSignal, packet);
        send(packet, "ifOut");
    }
    else {
        EV << "Dest address " << macAddressInd->getDestAddress() << " unknown, broadcasting frame " << packet << endl;
        broadcast(packet, arrivalInterfaceId);
    }
}

void MacRelayUnit::start()
{
}

void MacRelayUnit::stop()
{
}

void MacRelayUnit::learn(MacAddress srcAddr, int arrivalInterfaceId)
{
    macTable->updateTableWithAddress(arrivalInterfaceId, srcAddr);
}

void MacRelayUnit::finish()
{
    recordScalar("processed frames", numProcessedFrames);
    recordScalar("discarded frames", numDiscardedFrames);
}

} // namespace inet

