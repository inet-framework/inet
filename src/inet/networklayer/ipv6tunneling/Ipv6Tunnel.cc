//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv6tunneling/Ipv6Tunnel.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"

namespace inet {

Define_Module(Ipv6Tunnel);

void Ipv6Tunnel::initialize(int stage)
{
    LayeredProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        upperLayerInGateId = findGate("upperLayerIn");
        upperLayerOutGateId = findGate("upperLayerOut");
        source = Ipv6Address(par("source").stringValue());
        destination = Ipv6Address(par("destination").stringValue());
    }
    else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION)
        configureNetworkInterface();
}

void Ipv6Tunnel::configureNetworkInterface()
{
    networkInterface = getContainingNicModule(this);
    networkInterface->setMtu(par("mtu"));
    networkInterface->setPointToPoint(true);
}

void Ipv6Tunnel::handleUpperPacket(Packet *packet)
{
    // The packet is the inner datagram that IPv6 routed to this interface. Ask
    // the IPv6 layer to encapsulate it toward the tunnel endpoints: present it
    // to IPv6's *service* (upper) interface as a request (SP_REQUEST), so the
    // message dispatcher delivers it to the top of IPv6. IPv6 then wraps it in
    // an outer header (source -> destination, next header = IPv6, because the
    // payload protocol is ipv6) and routes the result as a locally-originated
    // datagram toward the exit -- no tunneling-specific code in the IPv6 core.
    packet->clearTags();
    auto addresses = packet->addTag<L3AddressReq>();
    addresses->setSrcAddress(source); // the tunnel entry point (RFC 2473)
    addresses->setDestAddress(destination);
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ipv6);
    auto req = packet->addTag<DispatchProtocolReq>();
    req->setProtocol(&Protocol::ipv6);
    req->setServicePrimitive(SP_REQUEST);
    send(packet, upperLayerOutGateId);
}

} // namespace inet
