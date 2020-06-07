#ifndef __INET_IPV4NATENTRY_H
#define __INET_IPV4NATENTRY_H

#include <omnetpp.h>



#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4NatEntry_m.h"
#include "inet/networklayer/ipv4/Ipv4NatTable.h"
#include "inet/transportlayer/common/L4Tools.h"

#ifdef WITH_TCP_COMMON
#include "inet/transportlayer/tcp_common/TcpCrcInsertionHook.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#endif

#ifdef WITH_UDP
#include "inet/transportlayer/udp/Udp.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"
#endif
#include "inet/common/INETDefs_m.h" // import inet.common.INETDefs

#include "inet/networklayer/contract/ipv4/Ipv4Address.h" // import inet.networklayer.contract.ipv4.Ipv4Address


namespace inet {

class Ipv4NatEntry : public Ipv4NatEntry_Base
{
public:
    using Ipv4NatEntry_Base::Ipv4NatEntry_Base;

    void applyToPacket(Packet *packet) const
    {
        const Ipv4NatEntry &natEntry = *this; // TODO remove

        auto& ipv4Header = removeNetworkProtocolHeader<Ipv4Header>(packet);
        if (!natEntry.getDestAddress().isUnspecified())
            ipv4Header->setDestAddress(natEntry.getDestAddress());
        if (!natEntry.getSrcAddress().isUnspecified())
            ipv4Header->setSrcAddress(natEntry.getSrcAddress());
        auto transportProtocol = ipv4Header->getProtocol();
    #ifdef WITH_UDP
        if (transportProtocol == &Protocol::udp) {
            auto& udpHeader = removeTransportProtocolHeader<UdpHeader>(packet);
            // TODO: if (!Udp::verifyCrc(Protocol::ipv4, udpHeader, packet))
            auto udpData = packet->peekData();
            if (natEntry.getDestPort() != -1)
                udpHeader->setDestPort(natEntry.getDestPort());
            if (natEntry.getSrcPort() != -1)
                udpHeader->setSrcPort(natEntry.getSrcPort());
            Udp::insertCrc(&Protocol::ipv4, ipv4Header->getSrcAddress(), ipv4Header->getDestAddress(), udpHeader, packet);
            insertTransportProtocolHeader(packet, Protocol::udp, udpHeader);
        }
        else
    #endif
    #ifdef WITH_TCP_COMMON
        if (transportProtocol == &Protocol::tcp) {
            auto& tcpHeader = removeTransportProtocolHeader<tcp::TcpHeader>(packet);
            // TODO: if (!Tcp::verifyCrc(Protocol::ipv4, tcpHeader, packet))
            auto tcpData = packet->peekData();
            if (natEntry.getDestPort() != -1)
                tcpHeader->setDestPort(natEntry.getDestPort());
            if (natEntry.getSrcPort() != -1)
                tcpHeader->setSrcPort(natEntry.getSrcPort());
            tcp::TcpCrcInsertion::insertCrc(&Protocol::ipv4, ipv4Header->getSrcAddress(), ipv4Header->getDestAddress(), tcpHeader, packet);
            insertTransportProtocolHeader(packet, Protocol::tcp, tcpHeader);
        }
        else
        if (transportProtocol == &Protocol::icmpv4) {
            // TODO
        }
        else
    #endif
            throw cRuntimeError("Unknown protocol: '%s'", transportProtocol ? transportProtocol->getName() : std::to_string((int)ipv4Header->getProtocolId()).c_str());
        insertNetworkProtocolHeader(packet, Protocol::ipv4, ipv4Header);
    }

};


} // namespace inet

#endif // ifndef __INET_IPV4NATENTRY_H

