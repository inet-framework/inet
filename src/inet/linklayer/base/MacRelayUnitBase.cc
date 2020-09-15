//
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/StringFormat.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/base/MacRelayUnitBase.h"

namespace inet {

Define_Module(MacRelayUnitBase);

void MacRelayUnitBase::initialize(int stage)
{
    LayeredProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        macAddressTable.set(this, "macTableModule");
        interfaceTable.set(this, "interfaceTableModule");
        numProcessedFrames = numDroppedFrames = 0;
        WATCH(numProcessedFrames);
        WATCH(numDroppedFrames);
    }
    else if (stage == INITSTAGE_LINK_LAYER)
        registerProtocol(Protocol::ethernetMac, gate("ifOut"), gate("ifIn"));
}

void MacRelayUnitBase::updateDisplayString() const
{
    auto text = StringFormat::formatString(par("displayStringTextFormat"), [&] (char directive) {
        static std::string result;
        switch (directive) {
            case 'p':
                result = std::to_string(numProcessedFrames);
                break;
            case 'd':
                result = std::to_string(numDroppedFrames);
                break;
            default:
                throw cRuntimeError("Unknown directive: %c", directive);
        }
        return result.c_str();
    });
    getDisplayString().setTagArg("t", 0, text);
}

void MacRelayUnitBase::broadcastPacket(Packet *outgoingPacket, const MacAddress& destinationAddress, NetworkInterface *incomingInterface)
{
    if (incomingInterface == nullptr)
        EV_INFO << "Broadcasting packet to all interfaces" << EV_FIELD(destinationAddress) << EV_FIELD(outgoingPacket) << EV_ENDL;
    else
        EV_INFO << "Broadcasting packet to all interfaces except incoming interface" << EV_FIELD(destinationAddress) << EV_FIELD(incomingInterface) << EV_FIELD(outgoingPacket) << EV_ENDL;
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        auto outgoingInterface = interfaceTable->getInterface(i);
        if (incomingInterface != outgoingInterface && isForwardingInterface(outgoingInterface))
            sendPacket(outgoingPacket->dup(), destinationAddress, outgoingInterface);
    }
    delete outgoingPacket;
}

void MacRelayUnitBase::sendPacket(Packet *packet, const MacAddress& destinationAddress, NetworkInterface *outgoingInterface)
{
    EV_INFO << "Sending packet to peer" << EV_FIELD(destinationAddress) << EV_FIELD(outgoingInterface) << EV_FIELD(packet) << EV_ENDL;
    packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(outgoingInterface->getInterfaceId());
    emit(packetSentToLowerSignal, packet);
    send(packet, "ifOut");
}

void MacRelayUnitBase::updatePeerAddress(NetworkInterface *incomingInterface, MacAddress sourceAddress)
{
    EV_INFO << "Learning peer address" << EV_FIELD(sourceAddress) << EV_FIELD(incomingInterface) << EV_ENDL;
    macAddressTable->updateTableWithAddress(incomingInterface->getInterfaceId(), sourceAddress);
}

void MacRelayUnitBase::finish()
{
    recordScalar("processed frames", numProcessedFrames);
    recordScalar("discarded frames", numDroppedFrames);
}

} // namespace inet

