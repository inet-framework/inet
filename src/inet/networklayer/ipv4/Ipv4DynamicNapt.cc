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

#include "inet/networklayer/ipv4/Ipv4DynamicNapt.h"

#include "inet/networklayer/ipv4/Ipv4NatTable.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
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

Define_Module(Ipv4DynamicNapt);

Ipv4DynamicNapt::~Ipv4DynamicNapt()
{
    delete incomingFilter;
    delete outgoingFilter;
}

void Ipv4DynamicNapt::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        networkProtocol = getModuleFromPar<INetfilter>(par("networkProtocolModule"), this);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {

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

void Ipv4DynamicNapt::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module can not handle messages");
}


class TransportPortsExtractor : public PacketDissector::CallbackBase {
public:
    const Protocol *transportProtocol = nullptr;
    uint16_t srcPort = -1;
    uint16_t destPort = -1;

    virtual void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) override {
        if (protocol == &Protocol::udp) {
            std::cout << "UDP!" << std::endl;
            transportProtocol = protocol;
            Ptr<const UdpHeader> header = dynamicPtrCast<const UdpHeader>(chunk);
            srcPort = header->getSrcPort();
            destPort = header->getDestPort();
        }
        if (protocol == &Protocol::tcp) {
            std::cout << "TCP!" << std::endl;
            transportProtocol = protocol;
            Ptr<const tcp::TcpHeader> header = dynamicPtrCast<const tcp::TcpHeader>(chunk);
            srcPort = header->getSrcPort();
            destPort = header->getDestPort();
        }
    }
};


INetfilter::IHook::Result Ipv4DynamicNapt::datagramPreRoutingHook(Packet *datagram)
{
    Enter_Method_Silent();

    if (incomingFilter->matches(datagram)) {

        TransportPortsExtractor cb;
        PacketDissector pd(ProtocolDissectorRegistry::globalRegistry, cb);
        pd.dissectPacket(datagram, &Protocol::ipv4);

        std::pair<int, uint16_t> transportProtocolAndDestPort = {cb.transportProtocol->getId(), cb.destPort};

        std::cout << "incoming packet, protocol id and dest port: " << transportProtocolAndDestPort.first << " " << transportProtocolAndDestPort.second << std::endl;
        if (portMapping.find(transportProtocolAndDestPort) != portMapping.end()) {
            auto mapTo = portMapping[transportProtocolAndDestPort];

            std::cout << "mapping to: " << mapTo.first << " " << mapTo.second << std::endl;

            Ipv4NatEntry natEntry;
            natEntry.setDestAddress(mapTo.first);
            natEntry.setDestPort(mapTo.second);
            natEntry.applyToPacket(datagram);
        }
        else {
            // this is mostly for ICMP, but is very much broken
            std::cout << "using hacky default mapping" << std::endl;
            Ipv4NatEntry natEntry;
            natEntry.setDestAddress(Ipv4Address("192.168.1.100"));
            natEntry.applyToPacket(datagram);
        }

    }

    return ACCEPT;
}


INetfilter::IHook::Result Ipv4DynamicNapt::datagramPostRoutingHook(Packet *datagram)
{
    Enter_Method_Silent();

    if (outgoingFilter->matches(datagram)) {

        // TODO don't do all this lookup for every packet...
        cModule *containingNode = findContainingNode(this);

        IInterfaceTable *ift = check_and_cast<IInterfaceTable *>(containingNode->getSubmodule("interfaceTable"));
        InterfaceEntry *ie = ift->findInterfaceByName(par("publicInterfaceName").stringValue());


        auto& ipv4Header = getNetworkProtocolHeader(datagram);
        auto transportProtocol = ipv4Header->getProtocol();

        std::cout << "outgoing packet transport protocol id: " << transportProtocol->getId() << std::endl;
        Ipv4Address sourceAddress = ipv4Header->getSourceAddress().toIpv4();
        uint16_t sourcePort = -1;

        TransportPortsExtractor cb;
        PacketDissector pd(ProtocolDissectorRegistry::globalRegistry, cb);
        pd.dissectPacket(datagram, &Protocol::ipv4);
        sourcePort = cb.srcPort;

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

        std::stringstream nattable;
        std::cout << "----" << std::endl;
        for (auto it = portMapping.begin(); it != portMapping.end(); ++it) {
            std::cout << Protocol::getProtocol(it->first.first)->getName() << ":" << it->first.second << " <-> " << it->second.first << ":" << it->second.second << std::endl;
            nattable << Protocol::getProtocol(it->first.first)->getName() << ": " << it->first.second << " <-> " << it->second.first << ":" << it->second.second << std::endl;
        }
        std::cout << "----" << std::endl;
        containingNode->getDisplayString().setTagArg("t", 0, nattable.str().c_str());
        containingNode->getDisplayString().setTagArg("t", 1, "r");

        Ipv4NatEntry natEntry;
        natEntry.setSrcAddress(ie->getIpv4Address());
        natEntry.setSrcPort(externalSourcePort);
        natEntry.applyToPacket(datagram);
    }

    return ACCEPT;
}



} // namespace inet

