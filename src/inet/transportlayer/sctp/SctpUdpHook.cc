//
// Copyright 2017 OpenSim Ltd.
//
// This library is free software, you can redistribute it and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 3 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#include "inet/common/INETDefs.h"
#include "inet/common/INETUtils.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/transportlayer/common/L4Tools.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"
#include "inet/transportlayer/sctp/SctpUdpHook.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {
namespace sctp {

INetfilter::IHook::Result SctpUdpHook::datagramPreRoutingHook(Packet *packet)
{
    EV_INFO << "SctpUdpHook::datagramPreRoutingHook\n";
    EV_INFO << "Packet: " << packet << endl;
 //   auto networkProtocol = packet->getMandatoryTag<PacketProtocolTag>()->getProtocol();
    const auto& networkHeader = getNetworkProtocolHeader(packet);
    EV_INFO << "Protocol is " << networkHeader->getProtocol()->getId() << endl;
    if (networkHeader->getProtocol() == &Protocol::udp) {
        EV_INFO << "Protocol is udp\n";
        const auto& ipv4Header = removeNetworkProtocolHeader<Ipv4Header>(packet);
        EV_INFO << "network header removed. packet now " << packet << endl;
        auto udpHeader = packet->peekAtFront<UdpHeader>();
        EV_INFO << "udp header removed. packet now " << packet << endl;
        EV_INFO << "dest port " << udpHeader->getDestPort() << endl;
        if (udpHeader->getDestPort() == SCTP_UDP_PORT) {
            packet->removeAtFront<UdpHeader>();
            ipv4Header->setProtocolId(IP_PROT_SCTP);
        }
        insertNetworkProtocolHeader(packet, Protocol::ipv4, ipv4Header);
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::sctp);
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::sctp);
        EV_INFO << "Packet mit Network: " << packet << endl;
    } else {
        EV_INFO << "Protocol is " << networkHeader->getProtocol()->getId() << endl;
    }
    return ACCEPT;
}

}   // namespace sctp
}   // namespace inet

