//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/protocol/ethernet/EthernetAddressInserter.h"
#include "inet/protocol/ethernet/EthernetHeaders_m.h"

namespace inet {

Define_Module(EthernetAddressInserter);

void EthernetAddressInserter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        interfaceEntry = getContainingNicModule(this);
}

void EthernetAddressInserter::processPacket(Packet *packet)
{
    const auto& header = makeShared<Ieee8023MacAddresses>();
    auto macAddressReq = packet->getTag<MacAddressReq>();
    auto srcAddress = macAddressReq->getSrcAddress();
    if (srcAddress.isUnspecified() && interfaceEntry != nullptr)
        srcAddress = interfaceEntry->getMacAddress();
    header->setSrc(srcAddress);
    header->setDest(macAddressReq->getDestAddress());
    packet->insertAtFront(header);
}

} // namespace inet

