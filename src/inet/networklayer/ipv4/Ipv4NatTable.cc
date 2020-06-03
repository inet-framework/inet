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

#ifdef WITH_TCP_COMMON
#include "inet/transportlayer/tcp_common/TcpCrcInsertionHook.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#endif

#ifdef WITH_UDP
#include "inet/transportlayer/udp/Udp.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"
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
        auto text = std::to_string(natEntries.size()) + " entries";
        getDisplayString().setTagArg("t", 0, text.c_str());
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
        // type
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
        // filter
        PacketFilter *packetFilter = new PacketFilter();
        const char *packetFilterAttr = xmlEntry->getAttribute("packetFilter");
        const char *packetDataFilterAttr = xmlEntry->getAttribute("packetDataFilter");
        packetFilter->setPattern(packetFilterAttr != nullptr ? packetFilterAttr : "*", packetDataFilterAttr != nullptr ? packetDataFilterAttr : "*");
        // NAT entry
        Ipv4NatEntry natEntry;
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
        // insert
        natEntries.insert({type, {packetFilter, natEntry}});
    }
}

void applyNatEntry(Packet *packet, const Ipv4NatEntry &natEntry)
{
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
            applyNatEntry(packet, natEntry);
            break;
        }
    }
    return ACCEPT;
}









Define_Module(Ipv4DynamicNat);

Ipv4DynamicNat::~Ipv4DynamicNat()
{
    delete incomingFilter;
    delete outgoingFilter;
}

void Ipv4DynamicNat::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        networkProtocol = getModuleFromPar<INetfilter>(par("networkProtocolModule"), this);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {

        // TODO: use privateSubnet parameter

        std::string privateSubnetAddress = par("privateSubnetAddress").stringValue();
        std::string privateSubnetNetmask = par("privateSubnetNetmask").stringValue();

        Ipv4Address addr(privateSubnetAddress.c_str());
        Ipv4Address mask(privateSubnetNetmask.c_str());
        int maskLength = mask.getNetmaskLength();

        if (!mask.isValidNetmask() || (maskLength != 8 && maskLength != 16 && maskLength != 24))
            throw cRuntimeError("privateSubnetNetmask must be one of: 255.255.255.0, 255.255.0.0, 255.0.0.0");

        addr = addr.doAnd(mask);

        std::string subnetFilter;
        switch (maskLength) {
            case 8:
                subnetFilter = std::to_string(addr.getDByte(0)) + ".*.*.*";
                break;
            case 16:
                subnetFilter = std::to_string(addr.getDByte(0)) + "." + std::to_string(addr.getDByte(1)) + ".*.*";
                break;
            case 24:
                subnetFilter = std::to_string(addr.getDByte(0)) + "." + std::to_string(addr.getDByte(1)) + "." + std::to_string(addr.getDByte(2)) + ".*";
                break;
        }

        outgoingFilter = new PacketFilter();
        outgoingFilter->setPattern("*", ("*Ipv4Header* AND sourceAddress =~ " + subnetFilter + " AND NOT destAddress =~ " + subnetFilter).c_str());

        incomingFilter = new PacketFilter(); // the last AND NOT destAddress =~ part is... strange... maybe unnecessary?
        incomingFilter->setPattern("*", ("*Ipv4Header* AND NOT sourceAddress =~ " + subnetFilter + " AND NOT destAddress =~ " + subnetFilter).c_str());

        networkProtocol->registerHook(0, this);
    }
}

void Ipv4DynamicNat::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module can not handle messages");
}

std::pair<int, uint16_t> getTransportProtocolAndDestPort(Packet *datagram)
{
    auto& ipv4Header = getNetworkProtocolHeader(datagram);
    auto transportProtocol = ipv4Header->getProtocol();

    int port = -1;

#ifdef WITH_UDP
    if (transportProtocol == &Protocol::udp) {
        auto& udpHeader = getTransportProtocolHeader(datagram);
        port = udpHeader->getDestinationPort();
    }
#endif

#ifdef WITH_TCP_COMMON
    if (transportProtocol == &Protocol::tcp) {
        auto& tcpHeader = getTransportProtocolHeader(datagram);
        port = tcpHeader->getDestinationPort();
    }
#endif

    return {transportProtocol->getId(), port};
}

INetfilter::IHook::Result Ipv4DynamicNat::datagramPreRoutingHook(Packet *datagram)
{
    Enter_Method_Silent();

    if (incomingFilter->matches(datagram)) {

        auto transportProtocolAndDestPort = getTransportProtocolAndDestPort(datagram);

        std::cout << "incoming packet, protocol id and dest port: " << transportProtocolAndDestPort.first << " " << transportProtocolAndDestPort.second << std::endl;
        if (portMapping.find(transportProtocolAndDestPort) != portMapping.end()) {
            auto mapTo = portMapping[transportProtocolAndDestPort];

            std::cout << "mapping to: " << mapTo.first << " " << mapTo.second << std::endl;

            Ipv4NatEntry natEntry;

            natEntry.setDestAddress(mapTo.first);
            natEntry.setDestPort(mapTo.second);

            applyNatEntry(datagram, natEntry);
        }
        else {
            std::cout << "using hacky default mapping" << std::endl;
            Ipv4NatEntry natEntry;
            natEntry.setDestAddress(Ipv4Address("192.168.1.100"));
            applyNatEntry(datagram, natEntry);
        }

    }

    return ACCEPT;
}

INetfilter::IHook::Result Ipv4DynamicNat::datagramPostRoutingHook(Packet *datagram)
{
    Enter_Method_Silent();

    if (outgoingFilter->matches(datagram)) {

        // TODO don't do all this lookup for every packet...

        IInterfaceTable *ift = check_and_cast<IInterfaceTable *>(findContainingNode(this)->getSubmodule("interfaceTable"));
        InterfaceEntry *ie = ift->findInterfaceByName(par("publicInterfaceName").stringValue());



        auto& ipv4Header = getNetworkProtocolHeader(datagram);
        auto transportProtocol = ipv4Header->getProtocol();

        std::cout << "outgoing packet transport protocol id: " << transportProtocol->getId() << std::endl;
        Ipv4Address sourceAddress = ipv4Header->getSourceAddress().toIpv4();
        uint16_t sourcePort = -1;

    #ifdef WITH_UDP
        if (transportProtocol == &Protocol::udp) {
            auto& udpHeader = getTransportProtocolHeader(datagram);
            sourcePort = udpHeader->getSourcePort();
        }
    #endif

    #ifdef WITH_TCP_COMMON
        if (transportProtocol == &Protocol::tcp) {
            auto& tcpHeader = getTransportProtocolHeader(datagram);
            sourcePort = tcpHeader->getSourcePort();
        }
    #endif

        std::pair<Ipv4Address, uint16_t> sourceAddressAndPort = {sourceAddress, sourcePort};

        std::cout << "outgoing packet, source address and port: " << sourceAddress << " " << sourcePort << std::endl;

        bool alreadyMapped = false;
        uint16_t externalSourcePort = -1;
        for (auto it = portMapping.begin(); it != portMapping.end(); ++it)
            if (it->second == sourceAddressAndPort) {
                alreadyMapped = true;
                externalSourcePort = it->first.second;
                break;
            }

        if (!alreadyMapped) {
            std::cout << "outgoing packet is not mapped yet" << std::endl;
            if (sourcePort != (uint16_t)-1) {
                static uint16_t nextExternalPort = 49000;
                nextExternalPort++;
                externalSourcePort = nextExternalPort;
                portMapping[{transportProtocol->getId(), nextExternalPort}] = sourceAddressAndPort;
            }
        }
        else {
            std::cout << "outgoing packet is already mapped" << std::endl;
        }
        std::cout << "----" << std::endl;
        for (auto it = portMapping.begin(); it != portMapping.end(); ++it)
            std::cout << "(" << it->first.first << ", " << it->first.second << " -> " << it->second.first << ", " << it->second.second << std::endl;
        std::cout << "----" << std::endl;
        Ipv4NatEntry natEntry;
        natEntry.setSrcAddress(ie->getIpv4Address());
        natEntry.setSrcPort(externalSourcePort);
        applyNatEntry(datagram, natEntry);
    }

    return ACCEPT;
}



} // namespace inet

