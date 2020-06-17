//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/INETDefs.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/XMLUtils.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/diffserv/DiffservUtil.h"
#include "inet/networklayer/diffserv/MultiFieldClassifier.h"
#include "inet/transportlayer/contract/TransportHeaderBase_m.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/Ipv6Header.h"
#endif // ifdef WITH_IPv6

#include "inet/transportlayer/common/L4Tools.h"

#ifdef WITH_TCP_COMMON
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#endif // ifdef WITH_TCP_COMMON

#ifdef WITH_UDP
#include "inet/transportlayer/udp/UdpHeader_m.h"
#endif // ifdef WITH_UDP

namespace inet {

using namespace DiffservUtil;

bool MultiFieldClassifier::PacketDissectorCallback::matches(const Packet *packet)
{
    matchesL3 = matchesL4 = false;
    dissect = true;
    const auto& packetProtocolTag = packet->findTag<PacketProtocolTag>();
    auto protocol = packetProtocolTag != nullptr ? packetProtocolTag->getProtocol() : nullptr;
    PacketDissector packetDissector(ProtocolDissectorRegistry::globalRegistry, *this);
    auto copy = packet->dup();
    packetDissector.dissectPacket(copy, protocol);
    delete copy;
    return matchesL3 && matchesL4;
}

bool MultiFieldClassifier::PacketDissectorCallback::shouldDissectProtocolDataUnit(const Protocol *protocol)
{
    return dissect;
}

void MultiFieldClassifier::PacketDissectorCallback::visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol)
{
    if (protocol == nullptr)
        return;
    if (*protocol == Protocol::ipv4) {
        dissect = false;
#ifdef WITH_IPv4
        const auto& ipv4Header = dynamicPtrCast<const Ipv4Header>(chunk);
        if (!ipv4Header)
            return;
        if (srcPrefixLength > 0 && (srcAddr.getType() != L3Address::IPv4 || !ipv4Header->getSrcAddress().prefixMatches(srcAddr.toIpv4(), srcPrefixLength)))
            return;
        if (destPrefixLength > 0 && (destAddr.getType() != L3Address::IPv4 || !ipv4Header->getDestAddress().prefixMatches(destAddr.toIpv4(), destPrefixLength)))
            return;
        if (protocolId >= 0 && protocolId != ipv4Header->getProtocolId())
            return;
        if (dscp != -1 && dscp != ipv4Header->getDscp())
            return;
        if (tos != -1 && tosMask != 0 && (tos & tosMask) != (ipv4Header->getTypeOfService() & tosMask))
            return;

        matchesL3 = true;
        if (srcPortMin < 0 && destPortMin < 0)
            matchesL4 = true;
        else
            dissect = true;
#endif // ifdef WITH_IPv4
    }
    else
    if (*protocol == Protocol::ipv6) {
#ifdef WITH_IPv6
        dissect = false;
        const auto& ipv6Header = dynamicPtrCast<const Ipv6Header>(chunk);
        if (!ipv6Header)
            return;

        if (srcPrefixLength > 0 && (srcAddr.getType() != L3Address::IPv6 || !ipv6Header->getSrcAddress().matches(srcAddr.toIpv6(), srcPrefixLength)))
            return;
        if (destPrefixLength > 0 && (destAddr.getType() != L3Address::IPv6 || !ipv6Header->getDestAddress().matches(destAddr.toIpv6(), destPrefixLength)))
            return;
        if (protocolId >= 0 && protocolId != ipv6Header->getProtocolId())
            return;
        if (dscp != -1 && dscp != ipv6Header->getDscp())
            return;
        if (tos != -1 && tosMask != 0 && (tos & tosMask) != (ipv6Header->getTrafficClass() & tosMask))
            return;

        matchesL3 = true;
        if (srcPortMin < 0 && destPortMin < 0)
            matchesL4 = true;
        else
            dissect = true;
#endif // ifdef WITH_IPv6
    }
    else
    if (isTransportProtocol(*protocol)) {
        const auto& transportHeader = dynamicPtrCast<const TransportHeaderBase>(chunk);
        if (!transportHeader)
            return;
        dissect = false;
        auto srcPort = transportHeader->getSourcePort();
        auto destPort = transportHeader->getDestinationPort();

        if (srcPortMin != -1 && (srcPort < (unsigned int)srcPortMin))
            return;
        if (srcPortMax != -1 && (srcPort > (unsigned int)srcPortMax))
            return;
        if (destPortMin != -1 && (destPort < (unsigned int)destPortMin))
            return;
        if (destPortMax != -1 && (destPort > (unsigned int)destPortMax))
            return;
        matchesL4 = true;
    }
}

Define_Module(MultiFieldClassifier);

simsignal_t MultiFieldClassifier::pkClassSignal = registerSignal("pkClass");

void MultiFieldClassifier::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        numOutGates = gateSize("out");

        numRcvd = 0;
        WATCH(numRcvd);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        cXMLElement *config = par("filters");
        configureFilters(config);
    }
}

void MultiFieldClassifier::pushPacket(Packet *packet, cGate *inputGate)
{
    numRcvd++;
    int gateIndex = classifyPacket(packet);
    emit(pkClassSignal, gateIndex);

    cGate *outputGate = nullptr;
    if (gateIndex >= 0)
        outputGate = gate("out", gateIndex);
    else
        outputGate = gate("defaultOut", gateIndex);
    auto consumer = findConnectedModule<IPassivePacketSink>(outputGate);
    pushOrSendPacket(packet, outputGate, consumer);
}

void MultiFieldClassifier::refreshDisplay() const
{
    char buf[20] = "";
    if (numRcvd > 0)
        sprintf(buf + strlen(buf), "rcvd:%d ", numRcvd);
    getDisplayString().setTagArg("t", 0, buf);
}

int MultiFieldClassifier::classifyPacket(Packet *packet)
{
    for (auto & elem : filters)
        if (elem.matches(packet))
            return elem.gateIndex;
    return -1;
}

void MultiFieldClassifier::addFilter(const PacketDissectorCallback& filter)
{
    if (filter.gateIndex < 0 || filter.gateIndex >= numOutGates)
        throw cRuntimeError("no output gate for gate index %d", filter.gateIndex);
    if (!filter.srcAddr.isUnspecified() && ((filter.srcAddr.getType() == L3Address::IPv6 && filter.srcPrefixLength > 128) ||
                                            (filter.srcAddr.getType() == L3Address::IPv4 && filter.srcPrefixLength > 32)))
        throw cRuntimeError("srcPrefixLength is invalid");
    if (!filter.destAddr.isUnspecified() && ((filter.destAddr.getType() == L3Address::IPv6 && filter.destPrefixLength > 128) ||
                                             (filter.destAddr.getType() == L3Address::IPv4 && filter.destPrefixLength > 32)))
        throw cRuntimeError("srcPrefixLength is invalid");
    if ((filter.protocolId < -1 || filter.protocolId > 0xff))
        throw cRuntimeError("protocol is not a valid protocol number");
    if (filter.dscp < -1 || filter.dscp > 0x3f)
        throw cRuntimeError("dscp is not valid");
    if (filter.tos < -1 || filter.tos > 0xff)
        throw cRuntimeError("tos is not valid");
    if (filter.tosMask < 0 || filter.tosMask > 0xff)
        throw cRuntimeError("tosMask is not valid");
    if (filter.srcPortMin < -1 || filter.srcPortMin > 0xffff)
        throw cRuntimeError("srcPortMin is not a valid port number");
    if (filter.srcPortMax < -1 || filter.srcPortMax > 0xffff)
        throw cRuntimeError("srcPortMax is not a valid port number");
    if (filter.srcPortMax != -1 && filter.srcPortMin > filter.srcPortMax)
        throw cRuntimeError("srcPortMin > srcPortMax");
    if (filter.destPortMin < -1 || filter.destPortMin > 0xffff)
        throw cRuntimeError("destPortMin is not a valid port number");
    if (filter.destPortMax < -1 || filter.destPortMax > 0xffff)
        throw cRuntimeError("destPortMax is not a valid port number");
    if (filter.destPortMax != -1 && filter.destPortMin > filter.destPortMax)
        throw cRuntimeError("destPortMin > destPortMax");

    filters.push_back(filter);
}

void MultiFieldClassifier::configureFilters(cXMLElement *config)
{
    L3AddressResolver addressResolver;
    cXMLElementList filterElements = config->getChildrenByTagName("filter");
    for (auto & filterElements_i : filterElements) {
        cXMLElement *filterElement = filterElements_i;
        try {
            const char *gateAttr = xmlutils::getMandatoryAttribute(*filterElement, "gate");
            const char *srcAddrAttr = filterElement->getAttribute("srcAddress");
            const char *srcPrefixLengthAttr = filterElement->getAttribute("srcPrefixLength");
            const char *destAddrAttr = filterElement->getAttribute("destAddress");
            const char *destPrefixLengthAttr = filterElement->getAttribute("destPrefixLength");
            const char *protocolAttr = filterElement->getAttribute("protocol");
            const char *dscpAttr = filterElement->getAttribute("dscp");
            const char *tosAttr = filterElement->getAttribute("tos");
            const char *tosMaskAttr = filterElement->getAttribute("tosMask");
            const char *srcPortAttr = filterElement->getAttribute("srcPort");
            const char *srcPortMinAttr = filterElement->getAttribute("srcPortMin");
            const char *srcPortMaxAttr = filterElement->getAttribute("srcPortMax");
            const char *destPortAttr = filterElement->getAttribute("destPort");
            const char *destPortMinAttr = filterElement->getAttribute("destPortMin");
            const char *destPortMaxAttr = filterElement->getAttribute("destPortMax");

            PacketDissectorCallback filter;
            filter.gateIndex = parseIntAttribute(gateAttr, "gate");
            if (srcAddrAttr)
                filter.srcAddr = addressResolver.resolve(srcAddrAttr);
            if (srcPrefixLengthAttr)
                filter.srcPrefixLength = parseIntAttribute(srcPrefixLengthAttr, "srcPrefixLength");
            else if (srcAddrAttr)
                filter.srcPrefixLength = filter.srcAddr.getType() == L3Address::IPv6 ? 128 : 32;
            if (destAddrAttr)
                filter.destAddr = addressResolver.resolve(destAddrAttr);
            if (destPrefixLengthAttr)
                filter.destPrefixLength = parseIntAttribute(destPrefixLengthAttr, "destPrefixLength");
            else if (destAddrAttr)
                filter.destPrefixLength = filter.destAddr.getType() == L3Address::IPv6 ? 128 : 32;
            if (protocolAttr)
                filter.protocolId = parseProtocol(protocolAttr, "protocol");
            if (dscpAttr)
                filter.dscp = parseIntAttribute(dscpAttr, "dscp");
            if (tosAttr)
                filter.tos = parseIntAttribute(tosAttr, "tos");
            if (tosMaskAttr)
                filter.tosMask = parseIntAttribute(tosMaskAttr, "tosMask");
            if (srcPortAttr)
                filter.srcPortMin = filter.srcPortMax = parseIntAttribute(srcPortAttr, "srcPort");
            if (srcPortMinAttr)
                filter.srcPortMin = parseIntAttribute(srcPortMinAttr, "srcPortMin");
            if (srcPortMaxAttr)
                filter.srcPortMax = parseIntAttribute(srcPortMaxAttr, "srcPortMax");
            if (destPortAttr)
                filter.destPortMin = filter.destPortMax = parseIntAttribute(destPortAttr, "destPort");
            if (destPortMinAttr)
                filter.destPortMin = parseIntAttribute(destPortMinAttr, "destPortMin");
            if (destPortMaxAttr)
                filter.destPortMax = parseIntAttribute(destPortMaxAttr, "destPortMax");

            addFilter(filter);
        }
        catch (std::exception& e) {
            throw cRuntimeError("Error in XML <filter> element at %s: %s", filterElement->getSourceLocation(), e.what());
        }
    }
}

} // namespace inet

