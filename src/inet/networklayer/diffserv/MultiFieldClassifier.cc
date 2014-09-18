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
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/IPv6Datagram.h"
#endif // ifdef WITH_IPv6

#ifdef WITH_UDP
#include "inet/transportlayer/udp/UDPPacket.h"
#endif // ifdef WITH_UDP

#ifdef WITH_TCP_COMMON
#include "inet/transportlayer/tcp_common/TCPSegment.h"
#endif // ifdef WITH_TCP_COMMON

#include "inet/networklayer/diffserv/MultiFieldClassifier.h"
#include "inet/networklayer/diffserv/DiffservUtil.h"

namespace inet {

using namespace DiffservUtil;

#ifdef WITH_IPv4
bool MultiFieldClassifier::Filter::matches(IPv4Datagram *datagram)
{
    if (srcPrefixLength > 0 && (srcAddr.getType() != L3Address::IPv4 || !datagram->getSrcAddress().prefixMatches(srcAddr.toIPv4(), srcPrefixLength)))
        return false;
    if (destPrefixLength > 0 && (destAddr.getType() != L3Address::IPv4 || !datagram->getDestAddress().prefixMatches(destAddr.toIPv4(), destPrefixLength)))
        return false;
    if (protocol >= 0 && datagram->getTransportProtocol() != protocol)
        return false;
    if (tosMask != 0 && (tos & tosMask) != (datagram->getTypeOfService() & tosMask))
        return false;
    if (srcPortMin >= 0 || destPortMin >= 0) {
        int srcPort = -1, destPort = -1;
        cPacket *packet = datagram->getEncapsulatedPacket();
#ifdef WITH_UDP
        UDPPacket *udpPacket = dynamic_cast<UDPPacket *>(packet);
        if (udpPacket) {
            srcPort = udpPacket->getSourcePort();
            destPort = udpPacket->getDestinationPort();
        }
#endif // ifdef WITH_UDP
#ifdef WITH_TCP_COMMON
        tcp::TCPSegment *tcpSegment = dynamic_cast<tcp::TCPSegment *>(packet);
        if (tcpSegment) {
            srcPort = tcpSegment->getSrcPort();
            destPort = tcpSegment->getDestPort();
        }
#endif // ifdef WITH_TCP_COMMON

        if (srcPortMin >= 0 && (srcPort < srcPortMin || srcPort > srcPortMax))
            return false;
        if (destPortMin >= 0 && (destPort < destPortMin || destPort > destPortMax))
            return false;
    }

    return true;
}

#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
bool MultiFieldClassifier::Filter::matches(IPv6Datagram *datagram)
{
    if (srcPrefixLength > 0 && (srcAddr.getType() != L3Address::IPv6 || !datagram->getSrcAddress().matches(srcAddr.toIPv6(), srcPrefixLength)))
        return false;
    if (destPrefixLength > 0 && (destAddr.getType() != L3Address::IPv6 || !datagram->getDestAddress().matches(destAddr.toIPv6(), destPrefixLength)))
        return false;
    if (protocol >= 0 && datagram->getTransportProtocol() != protocol)
        return false;
    if (tosMask != 0 && (tos & tosMask) != (datagram->getTrafficClass() & tosMask))
        return false;
    if (srcPortMin >= 0 || destPortMin >= 0) {
        int srcPort = -1, destPort = -1;
        cPacket *packet = datagram->getEncapsulatedPacket();
#ifdef WITH_UDP
        UDPPacket *udpPacket = dynamic_cast<UDPPacket *>(packet);
        if (udpPacket) {
            srcPort = udpPacket->getSourcePort();
            destPort = udpPacket->getDestinationPort();
        }
#endif // ifdef WITH_UDP
#ifdef WITH_TCP_COMMON
        tcp::TCPSegment *tcpSegment = dynamic_cast<tcp::TCPSegment *>(packet);
        if (tcpSegment) {
            srcPort = tcpSegment->getSrcPort();
            destPort = tcpSegment->getDestPort();
        }
#endif // ifdef WITH_TCP_COMMON

        if (srcPortMin >= 0 && (srcPort < srcPortMin || srcPort > srcPortMax))
            return false;
        if (destPortMin >= 0 && (destPort < destPortMin || destPort > destPortMax))
            return false;
    }

    return true;
}

#endif // ifdef WITH_IPv6

Define_Module(MultiFieldClassifier);

simsignal_t MultiFieldClassifier::pkClassSignal = registerSignal("pkClass");

void MultiFieldClassifier::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        numOutGates = gateSize("outs");

        numRcvd = 0;
        WATCH(numRcvd);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER_3) {
        cXMLElement *config = par("filters").xmlValue();
        configureFilters(config);
    }
}

void MultiFieldClassifier::handleMessage(cMessage *msg)
{
    cPacket *packet = check_and_cast<cPacket *>(msg);

    numRcvd++;
    int gateIndex = classifyPacket(packet);
    emit(pkClassSignal, gateIndex);

    if (gateIndex >= 0)
        send(packet, "outs", gateIndex);
    else
        send(packet, "defaultOut");

    if (ev.isGUI()) {
        char buf[20] = "";
        if (numRcvd > 0)
            sprintf(buf + strlen(buf), "rcvd:%d ", numRcvd);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

int MultiFieldClassifier::classifyPacket(cPacket *packet)
{
    for ( ; packet; packet = packet->getEncapsulatedPacket()) {
#ifdef WITH_IPv4
        IPv4Datagram *ipv4Datagram = dynamic_cast<IPv4Datagram *>(packet);
        if (ipv4Datagram) {
            for (std::vector<Filter>::iterator it = filters.begin(); it != filters.end(); ++it)
                if (it->matches(ipv4Datagram))
                    return it->gateIndex;

            return -1;
        }
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
        IPv6Datagram *ipv6Datagram = dynamic_cast<IPv6Datagram *>(packet);
        if (ipv6Datagram) {
            for (std::vector<Filter>::iterator it = filters.begin(); it != filters.end(); ++it)
                if (it->matches(ipv6Datagram))
                    return it->gateIndex;

            return -1;
        }
#endif // ifdef WITH_IPv6
    }

    return -1;
}

void MultiFieldClassifier::addFilter(const Filter& filter)
{
    if (filter.gateIndex < 0 || filter.gateIndex >= numOutGates)
        throw cRuntimeError("no output gate for gate index %d", filter.gateIndex);
    if (!filter.srcAddr.isUnspecified() && ((filter.srcAddr.getType() == L3Address::IPv6 && filter.srcPrefixLength > 128) ||
                                            (filter.srcAddr.getType() == L3Address::IPv4 && filter.srcPrefixLength > 32)))
        throw cRuntimeError("srcPrefixLength is invalid");
    if (!filter.destAddr.isUnspecified() && ((filter.destAddr.getType() == L3Address::IPv6 && filter.destPrefixLength > 128) ||
                                             (filter.destAddr.getType() == L3Address::IPv4 && filter.destPrefixLength > 32)))
        throw cRuntimeError("srcPrefixLength is invalid");
    if (filter.protocol != -1 && (filter.protocol < 0 || filter.protocol > 0xff))
        throw cRuntimeError("protocol is not a valid protocol number");
    if (filter.tos != -1 && (filter.tos < 0 || filter.tos > 0xff))
        throw cRuntimeError("tos is not valid");
    if (filter.tosMask < 0 || filter.tosMask > 0xff)
        throw cRuntimeError("tosMask is not valid");
    if (filter.srcPortMin != -1 && (filter.srcPortMin < 0 || filter.srcPortMin > 0xffff))
        throw cRuntimeError("srcPortMin is not a valid port number");
    if (filter.srcPortMax != -1 && (filter.srcPortMax < 0 || filter.srcPortMax > 0xffff))
        throw cRuntimeError("srcPortMax is not a valid port number");
    if (filter.srcPortMin != -1 && filter.srcPortMin > filter.srcPortMax)
        throw cRuntimeError("srcPortMin > srcPortMax");
    if (filter.destPortMin != -1 && (filter.destPortMin < 0 || filter.destPortMin > 0xffff))
        throw cRuntimeError("destPortMin is not a valid port number");
    if (filter.destPortMax != -1 && (filter.destPortMax < 0 || filter.destPortMax > 0xffff))
        throw cRuntimeError("destPortMax is not a valid port number");
    if (filter.destPortMin != -1 && filter.destPortMin > filter.destPortMax)
        throw cRuntimeError("destPortMin > destPortMax");

    filters.push_back(filter);
}

void MultiFieldClassifier::configureFilters(cXMLElement *config)
{
    L3AddressResolver addressResolver;
    cXMLElementList filterElements = config->getChildrenByTagName("filter");
    for (int i = 0; i < (int)filterElements.size(); i++) {
        cXMLElement *filterElement = filterElements[i];
        try {
            const char *gateAttr = getRequiredAttribute(filterElement, "gate");
            const char *srcAddrAttr = filterElement->getAttribute("srcAddress");
            const char *srcPrefixLengthAttr = filterElement->getAttribute("srcPrefixLength");
            const char *destAddrAttr = filterElement->getAttribute("destAddress");
            const char *destPrefixLengthAttr = filterElement->getAttribute("destPrefixLength");
            const char *protocolAttr = filterElement->getAttribute("protocol");
            const char *tosAttr = filterElement->getAttribute("tos");
            const char *tosMaskAttr = filterElement->getAttribute("tosMask");
            const char *srcPortAttr = filterElement->getAttribute("srcPort");
            const char *srcPortMinAttr = filterElement->getAttribute("srcPortMin");
            const char *srcPortMaxAttr = filterElement->getAttribute("srcPortMax");
            const char *destPortAttr = filterElement->getAttribute("destPort");
            const char *destPortMinAttr = filterElement->getAttribute("destPortMin");
            const char *destPortMaxAttr = filterElement->getAttribute("destPortMax");

            Filter filter;
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
                filter.protocol = parseProtocol(protocolAttr, "protocol");
            if (tosAttr)
                filter.tos = parseIntAttribute(tosAttr, "tos");
            if (tosMaskAttr)
                filter.tosMask = parseIntAttribute(tosAttr, "tosMask");
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

