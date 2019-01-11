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

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4NatTable.h"
#include "inet/transportlayer/common/L4Tools.h"

#ifdef WITH_UDP
#include "inet/transportlayer/udp/UdpHeader_m.h"
#include "inet/transportlayer/udp/Udp.h"
#endif

#ifdef WITH_TCP_COMMON
#include "inet/transportlayer/tcp_common/TcpCrcInsertionHook.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#endif

namespace inet {

Define_Module(Ipv4NatTable);

Ipv4NatTable::~Ipv4NatTable()
{
    for (auto& it : natEntries)
        delete it.second.first;
}

void Ipv4NatTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        config = par("config");
        networkProtocol = getModuleFromPar<INetfilter>(par("networkProtocolModule"), this);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        parseConfig();
        if (natEntries.size() != 0)
            networkProtocol->registerHook(0, this);
    }
}

void Ipv4NatTable::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module can not handle messages");
}

void Ipv4NatTable::parseConfig()
{
    cXMLElementList xmlEntries = config->getChildrenByTagName("entry");
    for (auto & xmlEntry : xmlEntries) {
        Ipv4NatEntry natEntry;
        PacketFilter *packetFilter = new PacketFilter();
        const char *packetFilterAttr = xmlEntry->getAttribute("packetFilter");
        const char *packetDataFilterAttr = xmlEntry->getAttribute("packetDataFilter");
        packetFilter->setPattern(packetFilterAttr != nullptr ? packetFilterAttr : "*", packetDataFilterAttr != nullptr ? packetDataFilterAttr : "*");
        const char *typeAttr = xmlEntry->getAttribute("type");
        INetfilter::IHook::Type type;
        if (!strcmp("prerouting", typeAttr))
            type = PREROUTING;
        else if (!strcmp("localin", typeAttr))
            type = LOCALIN;
        else if (!strcmp("forward", typeAttr))
            type = FORWARD;
        else if (!strcmp("postrouting", typeAttr))
            type = POSTROUTING;
        else if (!strcmp("localout", typeAttr))
            type = LOCALOUT;
        else
            throw cRuntimeError("Unknown type");
        const char *destAddressAttr = xmlEntry->getAttribute("destAddress");
        if (destAddressAttr != nullptr && *destAddressAttr != '\0')
            natEntry.setDestAddress(Ipv4Address(destAddressAttr));
        const char *destPortAttr = xmlEntry->getAttribute("destPort");
        if (destPortAttr != nullptr && *destPortAttr != '\0')
            natEntry.setDestPort(atoi(destPortAttr));
        const char *srcAddressAttr = xmlEntry->getAttribute("srcAddress");
        if (srcAddressAttr != nullptr && *srcAddressAttr != '\0')
            natEntry.setSrcAddress(Ipv4Address(srcAddressAttr));
        const char *srcPortAttr = xmlEntry->getAttribute("srcPort");
        if (srcPortAttr != nullptr && *srcPortAttr != '\0')
            natEntry.setSrcPort(atoi(srcPortAttr));
        natEntries.insert({type, {packetFilter, natEntry}});
    }
}

INetfilter::IHook::Result Ipv4NatTable::processPacket(Packet *packet, INetfilter::IHook::Type type)
{
    Enter_Method_Silent();
    auto lt = natEntries.lower_bound(type);
    auto ut = natEntries.upper_bound(type);
    for (; lt != ut; lt++) {
        const auto& packetFilter = lt->second.first;
        const auto& natEntry = lt->second.second;
        // TODO: this might be slow for too many filters
        if (packetFilter->matches(packet)) {
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
#endif
                throw cRuntimeError("Unknown protocol: '%s'", transportProtocol ? transportProtocol->getName() : std::to_string((int)ipv4Header->getProtocolId()).c_str());
            insertNetworkProtocolHeader(packet, Protocol::ipv4, ipv4Header);
            break;
        }
    }
    return ACCEPT;
}

} // namespace inet

