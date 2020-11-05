//
// Copyright (C) 2020 OpenSim Ltd and Marcel Marek
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

#include <algorithm>

#include "inet/networklayer/ipv4/ipsec/IPsec.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/INETUtils.h"
#include "inet/common/XMLUtils.h"
#include "inet/networklayer/ipv4/IPv4Datagram_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/tcp_common/TCPSegment.h"
#include "inet/transportlayer/udp/UDPPacket.h"
#include "IPsecAuthenticationHeader_m.h"
#include "IPsecEncapsulatingSecurityPayload_m.h"

using namespace inet::xmlutils;

namespace inet {
namespace ipsec {

simsignal_t IPsec::inProtectedAcceptSignal = registerSignal("inProtectedAccept");
simsignal_t IPsec::inProtectedDropSignal = registerSignal("inProtectedDrop");
simsignal_t IPsec::inUnprotectedBypassSignal = registerSignal("inUnprotectedBypass");
simsignal_t IPsec::inUnprotectedDropSignal = registerSignal("inUnprotectedDrop");

simsignal_t IPsec::outBypassSignal = registerSignal("outBypass");
simsignal_t IPsec::outProtectSignal = registerSignal("outProtect");
simsignal_t IPsec::outDropSignal = registerSignal("outDrop");

simsignal_t IPsec::inProcessDelaySignal = registerSignal("inProcessDelay");
simsignal_t IPsec::outProcessDelaySignal = registerSignal("outProcessDelay");

Define_Module(IPsec);

IPsec::IPsec()
{
}

IPsec::~IPsec()
{
}

void IPsec::initSecurityDBs(cXMLElement *spdConfig)
{
    for (cXMLElement *spdEntryElem : spdConfig->getChildrenByTagName("SecurityPolicy")) {
        SecurityPolicy *spdEntry = new SecurityPolicy();

        // Selector
        PacketSelector selector;
        parseSelector(getUniqueChild(spdEntryElem, "Selector"), selector);
        spdEntry->setSelector(selector);

        // Direction
        IPsecRule::Direction direction = parseDirection(getUniqueChild(spdEntryElem, "Direction"));
        spdEntry->setDirection(direction);

        // Action
        IPsecRule::Action action = parseAction(getUniqueChild(spdEntryElem, "Action"));
        spdEntry->setAction(action);

        if (action == IPsecRule::Action::PROTECT) {
            // Protection
            IPsecRule::Protection protection = parseProtection(getUniqueChild(spdEntryElem, "Protection"));
            spdEntry->setProtection(protection);

            // ICV length in bits
            int icvNumBits = xmlutils::getParameterIntValue(spdEntryElem, "IcvNumBits");
            spdEntry->setIcvNumBits(icvNumBits);

            // load SA details
            for (cXMLElement *saEntryElem : spdEntryElem->getChildrenByTagName("SecurityAssociation")) {
                // SPI
                const cXMLElement *spiElem = getUniqueChild(saEntryElem, "SPI");
                unsigned int spi = atoi(spiElem->getNodeValue());

                // add SAD entry
                SecurityAssociation *sadEntry = new SecurityAssociation();
                sadEntry->setRule(spdEntry->getRule());
                sadEntry->setSpi(spi);

                if (const cXMLElement *selectorElem = getUniqueChildIfExists(saEntryElem, "Selector")) {
                    IPsecRule rule = sadEntry->getRule();
                    PacketSelector selector = rule.getSelector();
                    parseSelector(selectorElem, selector); // allow overriding fields
                    rule.setSelector(selector);
                    sadEntry->setRule(rule);
                }

                sadModule->addEntry(sadEntry);
                spdEntry->addEntry(sadEntry);
            }
        }

        spdModule->addEntry(spdEntry);
    }
}

void IPsec::parseSelector(const cXMLElement *selectorElem, PacketSelector& selector)
{
    auto addrConv = [](std::string s) {return L3AddressResolver().resolve(s.c_str(), L3AddressResolver::ADDR_IPv4).toIPv4();};
    auto intConv = [](std::string s) {return atoi(s.c_str());};
    auto protocolConv = [](std::string s) {return parseProtocol(s);};
    if (const cXMLElement *localAddressElem = getUniqueChildIfExists(selectorElem, "LocalAddress"))
        selector.setLocalAddress(rangelist<IPv4Address>::parse(localAddressElem->getNodeValue(), addrConv));
    if (const cXMLElement *remoteAddressElem = getUniqueChildIfExists(selectorElem, "RemoteAddress"))
        selector.setRemoteAddress(rangelist<IPv4Address>::parse(remoteAddressElem->getNodeValue(), addrConv));
    if (const cXMLElement *protocolElem = getUniqueChildIfExists(selectorElem, "Protocol"))
        selector.setNextProtocol(rangelist<unsigned int>::parse(protocolElem->getNodeValue(), protocolConv));
    if (const cXMLElement *localPortElem = getUniqueChildIfExists(selectorElem, "LocalPort"))
        selector.setLocalPort(rangelist<unsigned int>::parse(localPortElem->getNodeValue(), intConv));
    if (const cXMLElement *remotePortElem = getUniqueChildIfExists(selectorElem, "RemotePort"))
        selector.setRemotePort(rangelist<unsigned int>::parse(remotePortElem->getNodeValue(), intConv));
    if (const cXMLElement *icmpTypeElem = getUniqueChildIfExists(selectorElem, "ICMPType"))
        selector.setIcmpType(rangelist<unsigned int>::parse(icmpTypeElem->getNodeValue(), intConv));
    if (const cXMLElement *icmpCodeElem = getUniqueChildIfExists(selectorElem, "ICMPCode"))
        selector.setIcmpCode(rangelist<unsigned int>::parse(icmpCodeElem->getNodeValue(), intConv));

    bool udpOrTcp = selector.getNextProtocol().contains(IP_PROT_TCP) || selector.getNextProtocol().contains(IP_PROT_UDP);
    bool portGiven = !selector.getLocalPort().empty() || !selector.getRemotePort().empty();
    if (portGiven && !udpOrTcp)
        throw cRuntimeError(selectorElem, "Ports are only accepted if protocol is TCP or UDP");
    if ((!selector.getIcmpType().empty() || !selector.getIcmpCode().empty()) && !selector.getNextProtocol().contains(IP_PROT_ICMP))
        throw cRuntimeError(selectorElem, "ICMPType and ICMPCode are only accepted if protocol is ICMP");
}

unsigned int IPsec::parseProtocol(const std::string& value)
{
    if (value == "TCP")
        return IP_PROT_TCP;
    else if (value == "UDP")
        return IP_PROT_UDP;
    else if (value == "ICMP")
        return IP_PROT_ICMP;
    else
        return inet::utils::atoul(value.c_str());
}

IPsecRule::Action IPsec::parseAction(const cXMLElement *elem)
{
    std::string value = elem->getNodeValue();
    if (value == "DISCARD")
        return IPsecRule::Action::DISCARD;
    else if (value == "BYPASS")
        return IPsecRule::Action::BYPASS;
    else if (value == "PROTECT")
        return IPsecRule::Action::PROTECT;
    else
        throw cRuntimeError(elem, "Rule has unknown action (DISCARD, BYPASS or PROTECT expected)");
}

IPsecRule::Direction IPsec::parseDirection(const cXMLElement *elem)
{
    std::string value = elem->getNodeValue();
    if (value == "IN")
        return IPsecRule::Direction::IN;
    else if (value == "OUT")
        return IPsecRule::Direction::OUT;
    else
        throw cRuntimeError(elem, "Rule has unknown direction (IN or OUT expected)");
}

IPsecRule::Protection IPsec::parseProtection(const cXMLElement *elem)
{
    std::string value = elem->getNodeValue();
    if (value == "AH")
        return IPsecRule::Protection::AH;
    else if (value == "ESP")
        return IPsecRule::Protection::ESP;
    else
        throw cRuntimeError(elem, "Unknown protection (AH, ESP or AH_ESP expected)");
}

void IPsec::initialize(int stage)
{
    if (stage == INITSTAGE_NETWORK_LAYER_3) {
        ahProtectOutDelay = par("ahProtectOutDelay");
        ahProtectInDelay = par("ahProtectInDelay");

        espProtectOutDelay = par("espProtectOutDelay");
        espProtectInDelay = par("espProtectInDelay");

        ipLayer = getModuleFromPar<IPv4>(par("networkProtocolModule"), this);

        //register IPsec hook
        ipLayer->registerHook(0, this);

        spdModule = getModuleFromPar<SecurityPolicyDatabase>(par("spdModule"), this);
        sadModule = getModuleFromPar<SecurityAssociationDatabase>(par("sadModule"), this);

        cXMLElement *spdConfig = par("spdConfig").xmlValue();

        initSecurityDBs(spdConfig);
    }
}

void IPsec::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        INetworkDatagram *context = static_cast<INetworkDatagram *>(msg->getContextPointer());
        delete msg;
        ipLayer->reinjectQueuedDatagram(context);
    }
}

INetfilter::IHook::Result IPsec::datagramPreRoutingHook(INetworkDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr)
{
    // First hook at the receiver (before fragment reassembly)
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result IPsec::datagramForwardHook(INetworkDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr)
{
    return INetfilter::IHook::ACCEPT;
}

PacketInfo IPsec::extractEgressPacketInfo(IPv4Datagram *ipv4datagram, const IPv4Address& localAddress)
{
    PacketInfo packetInfo;
    packetInfo.setLocalAddress(localAddress);
    packetInfo.setRemoteAddress(ipv4datagram->getDestinationAddress().toIPv4());

    packetInfo.setNextProtocol(ipv4datagram->getTransportProtocol());

    if (ipv4datagram->getTransportProtocol() == IP_PROT_TCP) {
        tcp::TCPSegment *tcpSegment = check_and_cast<tcp::TCPSegment *>(ipv4datagram->getEncapsulatedPacket());
        packetInfo.setLocalPort(tcpSegment->getSourcePort());
        packetInfo.setRemotePort(tcpSegment->getDestinationPort());
    }
    else if (ipv4datagram->getTransportProtocol() == IP_PROT_UDP) {
        UDPPacket *udpPacket = check_and_cast<UDPPacket *>(ipv4datagram->getEncapsulatedPacket());
        packetInfo.setLocalPort(udpPacket->getSourcePort());
        packetInfo.setRemotePort(udpPacket->getDestinationPort());
    }
    else if (ipv4datagram->getTransportProtocol() == IP_PROT_ICMP) {
        ICMPMessage *icmpMessage = check_and_cast<ICMPMessage *>(ipv4datagram->getEncapsulatedPacket());
        packetInfo.setIcmpType(icmpMessage->getType());
        packetInfo.setIcmpCode(icmpMessage->getCode());
    }
    return packetInfo;
}

INetfilter::IHook::Result IPsec::datagramPostRoutingHook(INetworkDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr)
{
    return processEgressPacket(check_and_cast<IPv4Datagram *>(datagram), outIE->getIPv4Address());
}

INetfilter::IHook::Result IPsec::processEgressPacket(IPv4Datagram *ipv4datagram, const IPv4Address& localAddress)
{
    // packet will be fragmented (if necessary) later

    PacketInfo egressPacketInfo = extractEgressPacketInfo(ipv4datagram, localAddress);

    //search Security Policy Database
    SecurityPolicy *spdEntry = spdModule->findEntry(IPsecRule::Direction::OUT, &egressPacketInfo);
    if (spdEntry != nullptr) {
        if (spdEntry->getAction() == IPsecRule::Action::PROTECT) {
            emit(outProtectSignal, 1L);
            outProtect++;
            return protectDatagram(ipv4datagram, egressPacketInfo, spdEntry);
        }
        else if (spdEntry->getAction() == IPsecRule::Action::BYPASS) {
            EV_INFO << "IPsec OUT BYPASS rule, packet: " << egressPacketInfo.str() << std::endl;
            emit(outBypassSignal, 1L);
            outBypass++;
            return INetfilter::IHook::ACCEPT;
        }
        else if (spdEntry->getAction() == IPsecRule::Action::DISCARD) {
            EV_INFO << "IPsec OUT DROP rule, packet: " << egressPacketInfo.str() << std::endl;
            emit(outDropSignal, 1L);
            outDrop++;
            return INetfilter::IHook::DROP;
        }
    }
    EV_INFO << "IPsec OUT BYPASS, no matching rule, packet: " << egressPacketInfo.str() << std::endl;
    emit(outDropSignal, 1L);
    outDrop++;
    return INetfilter::IHook::DROP;
}

PacketInfo IPsec::extractIngressPacketInfo(IPv4Datagram *ipv4datagram)
{
    PacketInfo packetInfo;

    packetInfo.setLocalAddress(ipv4datagram->getDestinationAddress().toIPv4());
    packetInfo.setRemoteAddress(ipv4datagram->getSourceAddress().toIPv4());

    packetInfo.setNextProtocol(ipv4datagram->getTransportProtocol());

    if (ipv4datagram->getTransportProtocol() == IP_PROT_TCP) {
        tcp::TCPSegment *tcpSegment = check_and_cast<tcp::TCPSegment *>(ipv4datagram->getEncapsulatedPacket());
        packetInfo.setLocalPort(tcpSegment->getDestinationPort());
        packetInfo.setRemotePort(tcpSegment->getSourcePort());
    }
    else if (ipv4datagram->getTransportProtocol() == IP_PROT_UDP) {
        UDPPacket *udpPacket = check_and_cast<UDPPacket *>(ipv4datagram->getEncapsulatedPacket());
        packetInfo.setLocalPort(udpPacket->getDestinationPort());
        packetInfo.setRemotePort(udpPacket->getSourcePort());
    }
    else if (ipv4datagram->getTransportProtocol() == IP_PROT_ICMP) {
        ICMPMessage *icmpMessage = check_and_cast<ICMPMessage *>(ipv4datagram->getEncapsulatedPacket());
        packetInfo.setIcmpType(icmpMessage->getType());
        packetInfo.setIcmpCode(icmpMessage->getCode());
    }
    return packetInfo;
}

cPacket *IPsec::espProtect(cPacket *transport, SecurityAssociation *sadEntry, int transportType)
{
    IPsecEncapsulatingSecurityPayload *espPacket = new IPsecEncapsulatingSecurityPayload();
    unsigned int blockSize = 16;
    unsigned int pad = (blockSize - (transport->getByteLength() + 2) % blockSize) % blockSize; //TODO revise
    unsigned int icv = 4 * ((sadEntry->getIcvNumBits() + 31) / 32);
    unsigned int len = pad + 2 + IP_ESP_HEADER_BYTES + icv;
    espPacket->setByteLength(len);

    espPacket->setSequenceNumber(sadEntry->getAndIncSeqNum());
    espPacket->setSpi(sadEntry->getSpi());
    espPacket->encapsulate(transport);
    espPacket->setNextHeader(transportType);

    return espPacket;
}

cPacket *IPsec::ahProtect(cPacket *transport, SecurityAssociation *sadEntry, int transportType)
{
    IPsecAuthenticationHeader *ahPacket = new IPsecAuthenticationHeader();
    unsigned int icv = 4 * ((sadEntry->getIcvNumBits() + 31) / 32);
    unsigned int len = IP_AH_HEADER_BYTES + icv;
    ahPacket->setByteLength(len);

    ahPacket->setSequenceNumber(sadEntry->getAndIncSeqNum());
    ahPacket->setSpi(sadEntry->getSpi());
    ahPacket->encapsulate(transport);
    ahPacket->setNextHeader(transportType);

    return ahPacket;
}

INetfilter::IHook::Result IPsec::protectDatagram(IPv4Datagram *ipv4datagram, const PacketInfo& packetInfo, SecurityPolicy *spdEntry)
{
    Enter_Method_Silent();
    cPacket *transport = ipv4datagram->decapsulate();
    double delay = 0;

    for (auto saEntry : spdEntry->getEntries()) {
        if (saEntry->getRule().getSelector().matches(&packetInfo)) {
            if (saEntry->getProtection() == IPsecRule::Protection::ESP) {
                EV_INFO << "IPsec OUT ESP PROTECT packet: " << packetInfo.str() << std::endl;

                int transportType = ipv4datagram->getTransportProtocol();
                ipv4datagram->setTransportProtocol(IP_PROT_ESP);
                transport = espProtect(transport, (saEntry), transportType);

                delay += espProtectOutDelay;
            }
            else if (saEntry->getProtection() == IPsecRule::Protection::AH) {
                EV_INFO << "IPsec OUT AH PROTECT packet: " << packetInfo.str() << std::endl;

                int transportType = ipv4datagram->getTransportProtocol();
                ipv4datagram->setTransportProtocol(IP_PROT_AH);
                transport = ahProtect(transport, saEntry, transportType);

                delay += ahProtectOutDelay;
            }
        }
    }

    ipv4datagram->encapsulate(transport);

    if (delay > 0 || lastProtectedOut > simTime()) {
        cMessage *selfmsg = new cMessage("IPsecProtectOutDelay");
        selfmsg->setContextPointer(static_cast<INetworkDatagram *>(ipv4datagram));
        lastProtectedOut = std::max(simTime(), lastProtectedOut) + delay;
        scheduleAt(lastProtectedOut, selfmsg);

        simtime_t actualDelay = lastProtectedOut - simTime();
        EV_INFO << "IPsec OUT PROTECT (delaying by: " << actualDelay << "s), packet: " << packetInfo.str() << std::endl;
        emit(outProcessDelaySignal, actualDelay.dbl());

        return INetfilter::IHook::QUEUE;
    }
    else {
        EV_INFO << "IPsec OUT PROTECT packet: " << packetInfo.str() << std::endl;
        emit(outProcessDelaySignal, 0.0);
        return INetfilter::IHook::ACCEPT;
    }
}

INetfilter::IHook::Result IPsec::datagramLocalInHook(INetworkDatagram *datagram, const InterfaceEntry *inIE)
{
    return processIngressPacket(check_and_cast<IPv4Datagram *>(datagram));
}

INetfilter::IHook::Result IPsec::processIngressPacket(IPv4Datagram *ipv4datagram)
{
    Enter_Method_Silent();

    int transportProtocol = ipv4datagram->getTransportProtocol();

    PacketInfo ingressPacketInfo = extractIngressPacketInfo(ipv4datagram);

    double delay = 0.0;
    if (transportProtocol == IP_PROT_AH) {
        IPsecAuthenticationHeader *ah = check_and_cast<IPsecAuthenticationHeader *>(ipv4datagram->decapsulate());

        // find SA Entry in SAD
        SecurityAssociation *sadEntry = sadModule->findEntry(IPsecRule::Direction::IN, ah->getSpi());

        if (sadEntry != nullptr) {
            if (sadEntry->getProtection() != IPsecRule::Protection::AH) {
                EV_INFO << "IPsec IN DROP AH, SA does not prescribe AH, packet: " << ingressPacketInfo.str() << std::endl;
                emit(inProtectedDropSignal, 1L);
                inDrop++;
                return INetfilter::IHook::DROP;
            }

            // found SA
            ipv4datagram->encapsulate(ah->decapsulate());
            ipv4datagram->setTransportProtocol(ah->getNextHeader());

            delay = ahProtectInDelay;

            if (ah->getNextHeader() != IP_PROT_ESP) {
                delete ah;

                emit(inProtectedAcceptSignal, 1L);
                inAccept++;

                if (delay > 0 || lastProtectedIn > simTime()) {
                    cMessage *selfmsg = new cMessage("IPsecProtectInDelay");
                    selfmsg->setContextPointer(static_cast<INetworkDatagram*>(ipv4datagram));
                    lastProtectedIn = std::max(simTime(), lastProtectedIn) + delay;
                    scheduleAt(lastProtectedIn, selfmsg);
                    simtime_t actualDelay = lastProtectedIn - simTime();

                    EV_INFO << "IPsec IN ACCEPT AH (delaying by: " << actualDelay << "s), packet: " << ingressPacketInfo.str() << std::endl;
                    emit(inProcessDelaySignal, actualDelay.dbl());
                    return INetfilter::IHook::QUEUE;
                }
                else {
                    EV_INFO << "IPsec IN ACCEPT AH, packet: " << ingressPacketInfo.str() << std::endl;
                    emit(inProcessDelaySignal, 0.0);
                    return INetfilter::IHook::ACCEPT;
                }
            }
        }
        delete ah;

        if (sadEntry == nullptr) {
            EV_INFO << "IPsec IN DROP AH, no matching rule, packet: " << ingressPacketInfo.str() << std::endl;
            emit(inProtectedDropSignal, 1L);
            inDrop++;
            return INetfilter::IHook::DROP;
        }
    }

    transportProtocol = ipv4datagram->getTransportProtocol();

    if (transportProtocol == IP_PROT_ESP) {
        IPsecEncapsulatingSecurityPayload *esp = check_and_cast<IPsecEncapsulatingSecurityPayload *>(ipv4datagram->decapsulate());

        SecurityAssociation *sadEntry = sadModule->findEntry(IPsecRule::Direction::IN, esp->getSpi());

        if (sadEntry != nullptr) {
            if (sadEntry->getProtection() != IPsecRule::Protection::ESP) {
                EV_INFO << "IPsec IN DROP ESP, SA does not prescribe ESP, packet: " << ingressPacketInfo.str() << std::endl;
                emit(inProtectedDropSignal, 1L);
                inDrop++;
                return INetfilter::IHook::DROP;
            }

            // found SA
            emit(inProtectedAcceptSignal, 1L);
            inAccept++;

            ipv4datagram->encapsulate(esp->decapsulate());
            ipv4datagram->setTransportProtocol(esp->getNextHeader());
            delete esp;

            delay += espProtectInDelay;

            if (delay > 0 || lastProtectedIn > simTime()) {
                cMessage *selfmsg = new cMessage("IPsecProtectInDelay");
                selfmsg->setContextPointer(static_cast<INetworkDatagram*>(ipv4datagram));
                lastProtectedOut = std::max(simTime(), lastProtectedOut) + delay;
                scheduleAt(lastProtectedOut, selfmsg);
                simtime_t actualDelay = lastProtectedOut - simTime();

                EV_INFO << "IPsec IN ACCEPT ESP (delaying by: " << actualDelay << "s), packet: " << ingressPacketInfo.str() << std::endl;
                emit(inProcessDelaySignal, actualDelay.dbl());
                return INetfilter::IHook::QUEUE;
            }
            else {
                EV_INFO << "IPsec IN ACCEPT ESP, packet: " << ingressPacketInfo.str() << std::endl;
                emit(inProcessDelaySignal, 0.0);
                return INetfilter::IHook::ACCEPT;
            }
        }

        delete esp;

        if (sadEntry == nullptr) {
            EV_INFO << "IPsec IN DROP ESP, no matching rule, packet: " << ingressPacketInfo.str() << std::endl;
            emit(inProtectedDropSignal, 1L);
            inDrop++;
            return INetfilter::IHook::DROP;
        }
    }

    SecurityPolicy *spdEntry = spdModule->findEntry(IPsecRule::Direction::IN, &ingressPacketInfo);
    if (spdEntry != nullptr) {
        if (spdEntry->getAction() == IPsecRule::Action::BYPASS) {
            EV_INFO << "IPsec BYPASS rule, packet: " << ingressPacketInfo.str() << std::endl;
            emit(inUnprotectedBypassSignal, 1L);
            inBypass++;
            return INetfilter::IHook::ACCEPT;
        }
        else if (spdEntry->getAction() == IPsecRule::Action::DISCARD) {
            EV_INFO << "IPsec DROP rule, packet: " << ingressPacketInfo.str() << std::endl;
            emit(inUnprotectedDropSignal, 1L);
            inDrop++;
            return INetfilter::IHook::DROP;
        }
        else {
            EV_INFO << "IPsec IN DROP due to wrong rule, packet: " << ingressPacketInfo.str() << std::endl;
            emit(inUnprotectedDropSignal, 1L);
            inDrop++;
            return INetfilter::IHook::DROP;

        }
    }

    EV_INFO << "IPsec IN DROP, no matching rule, packet: " << ingressPacketInfo.str() << std::endl;
    emit(inUnprotectedDropSignal, 1L);
    inDrop++;
    return INetfilter::IHook::DROP;
}

INetfilter::IHook::Result IPsec::datagramLocalOutHook(INetworkDatagram *datagram, const InterfaceEntry *& outIE, L3Address& nextHopAddr)
{
    return INetfilter::IHook::ACCEPT;
}

void IPsec::refreshDisplay() const
{
    char buf[80];
    sprintf(buf, "IN: ACCEPT: %d DROP: %d BYPASS: %d\n OUT: PROTECT: %d DROP: %d BYPASS: %d", inAccept, inDrop, inBypass, outProtect, outDrop, outBypass);
    getDisplayString().setTagArg("t", 0, buf);
}

}    // namespace ipsec
}    // namespace inet

