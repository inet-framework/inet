//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/forwarding/Forwarding.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/NextHopAddressTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/selectivity/DestinationL3AddressHeader_m.h"

namespace inet {

Define_Module(Forwarding);

void Forwarding::initialize(int stage)
{
    PacketPusherBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *addressAsString = par("address");
        if (strlen(addressAsString) != 0)
            address = L3AddressResolver().resolve(addressAsString);
        if (strlen(par("interfaceTableModule").stringValue()) != 0)
            interfaceTable.reference(this, "interfaceTableModule", true);
        parseRoutes(par("routes"));
        registerService(AccessoryProtocol::forwarding, inputGate, inputGate);
        registerProtocol(AccessoryProtocol::forwarding, outputGate, outputGate);
    }
}

// Parses a "destination nextHop [interface]; ..." string into the routing table. Each
// entry is a whitespace-separated triple; the interface field is optional (defaults to -1,
// meaning "no explicit outgoing interface").
void Forwarding::parseRoutes(const char *routesString)
{
    cStringTokenizer entryTokenizer(routesString, ";");
    while (entryTokenizer.hasMoreTokens()) {
        std::vector<std::string> fields = cStringTokenizer(entryTokenizer.nextToken()).asVector();
        if (fields.empty())
            continue;
        if (fields.size() < 2)
            throw cRuntimeError("%s: invalid route entry, expected 'destination nextHop [interface]'", getFullPath().c_str());
        Route route;
        route.destination = L3AddressResolver().resolve(fields[0].c_str());
        route.nextHop = L3AddressResolver().resolve(fields[1].c_str());
        route.interface = fields.size() >= 3 ? atoi(fields[2].c_str()) : -1;
        routes.push_back(route);
    }
}

std::pair<L3Address, int> Forwarding::findNextHop(const L3Address& destinationAddress) const
{
    for (auto& route : routes)
        if (route.destination == destinationAddress)
            return { route.nextHop, route.interface };
    return { L3Address(), -1 };
}

int Forwarding::resolveInterfaceId(int interface) const
{
    if (interface == -1)
        return -1;
    if (interfaceTable != nullptr)
        return interfaceTable->getInterface(interface)->getInterfaceId();
    return interface;
}

void Forwarding::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    // whatever routed the packet here (e.g. a dispatch request) is stale now; the outcome below sets a fresh one
    packet->removeTagIfPresent<DispatchProtocolReq>();
    auto header = packet->peekAtFront<DestinationL3AddressHeader>();
    auto destinationAddress = header->getDestinationAddress();
    if (!address.isUnspecified() && destinationAddress == address) {
        // addressed to us: hand it up for local delivery. The DestinationL3AddressHeader is
        // left in place -- ReceiveAtL3Address is what checks and pops it.
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&AccessoryProtocol::destinationL3Address);
        pushOrSendPacket(packet, outputGate, consumer);
    }
    else {
        auto nextHop = findNextHop(destinationAddress);
        if (nextHop.first.isUnspecified()) {
            EV_WARN << "No route to " << destinationAddress << ", dropping packet " << packet->getName() << endl;
            dropPacket(packet, NO_ROUTE_FOUND);
        }
        else {
            int interfaceId = resolveInterfaceId(nextHop.second);
            if (interfaceId != -1)
                packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(interfaceId);
            packet->addTagIfAbsent<NextHopAddressReq>()->setNextHopAddress(nextHop.first);
            packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&AccessoryProtocol::forwarding);
            packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&AccessoryProtocol::hopLimit);
            pushOrSendPacket(packet, outputGate, consumer);
        }
    }
}

} // namespace inet

