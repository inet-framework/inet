//
// Copyright (C) 2011 OpenSim Ltd.
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

#include "inet/linklayer/ethernet/common/EthernetForker.h"

#include "inet/common/DirectionTag_m.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(EthernetForker);

void EthernetForker::initialize(int stage)
{
    PacketClassifierBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        interfaceTable.reference(this, "interfaceTableModule", true);
        if (!par("bridgeAddress").isEmptyString())
            bridgeAddress.setAddress(par("bridgeAddress").stringValue());
        auto localDeliveryMacAddresses = check_and_cast<cValueArray*>(par("localDeliveryMacAddresses").objectValue())->asObjectVector<cValueArray>();
        for (auto elem : localDeliveryMacAddresses) {
            if (elem->size() == 1) {
                MacAddress addr(elem->get(0).stringValue());
                registeredMacAddresses.insert(MacAddressPair(addr, addr));
            }
            else if (elem->size() == 2) {
                MacAddress startAddr(elem->get(0).stringValue());
                MacAddress endAddr(elem->get(1).stringValue());
                registeredMacAddresses.insert(MacAddressPair(startAddr, endAddr));
            }
            else
                throw cRuntimeError("Need one or two MAC address in one elem.");
        }

        WATCH(bridgeAddress);
    }
}

int EthernetForker::classifyPacket(Packet *packet)
{
    auto directionTag = packet->findTag<DirectionTag>();
    if (directionTag != nullptr && directionTag->getDirection() != DIRECTION_INBOUND)
        throw cRuntimeError("Packet must be inbound");

    auto interfaceInd = packet->getTag<InterfaceInd>();
    auto macAddressInd = packet->getTag<MacAddressInd>();
    auto incomingInterfaceId = interfaceInd->getInterfaceId();
    auto incomingInterface = interfaceTable->getInterfaceById(incomingInterfaceId);
    auto sourceAddress = macAddressInd->getSrcAddress();
    auto destinationAddress = macAddressInd->getDestAddress();

    if (destinationAddress.isBroadcast())
        return 1;   // TODO send up + forward?
    if (destinationAddress == bridgeAddress)
        return 0;
    if (in_range(registeredMacAddresses, destinationAddress))
        return 0;
    if (incomingInterface->matchesMacAddress(destinationAddress))
        return 0;
    return 1;
}

std::vector<cGate *> EthernetForker::getRegistrationForwardingGates(cGate *gate)
{
    if (gate == inputGate)
        return outputGates;
    else if (contains(outputGates, gate))
        return std::vector<cGate *>({inputGate});
    else
        throw cRuntimeError("Unknown gate");
}

} // namespace inet

