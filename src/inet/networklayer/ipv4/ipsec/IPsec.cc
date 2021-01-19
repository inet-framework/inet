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
    const char *s;
    Protection defaultProtection = strlen(s=par("defaultProtection"))==0 ? (Protection)-1 : protectionEnum.valueFor(s);
    EspMode defaultEspMode = strlen(s=par("defaultEspMode"))==0 ? (EspMode)-1 : espModeEnum.valueFor(s);
    EncryptionAlg defaultEncryptionAlg = strlen(s=par("defaultEncryptionAlg"))==0 ? (EncryptionAlg)-1 : encryptionAlgEnum.valueFor(s);
    AuthenticationAlg defaultAuthenticationAlg = strlen(s=par("defaultAuthenticationAlg"))==0 ? (AuthenticationAlg)-1 : authenticationAlgEnum.valueFor(s);
    int defaultMaxTfcPadLength = par("defaultMaxTfcPadLength").intValue();

    checkTags(spdConfig, "SecurityPolicy");

    for (cXMLElement *spdEntryElem : spdConfig->getChildrenByTagName("SecurityPolicy")) {
        SecurityPolicy *spdEntry = new SecurityPolicy();

        checkTags(spdEntryElem, "Selector Direction Action Protection EspMode EncryptionAlg AuthenticationAlg MaxTfcPadLength SecurityAssociation");

        // Selector
        PacketSelector selector;
        parseSelector(getUniqueChild(spdEntryElem, "Selector"), selector);
        spdEntry->setSelector(selector);

        // Direction
        Direction direction = parseEnumElem(directionEnum, spdEntryElem, "Direction");
        spdEntry->setDirection(direction);

        // Action
        Action action = parseEnumElem(actionEnum, spdEntryElem, "Action");
        spdEntry->setAction(action);

        if (action == Action::PROTECT) {
            // Protection
            Protection protection = parseEnumElem(protectionEnum, spdEntryElem, "Protection", defaultProtection);
            spdEntry->setProtection(protection);

            if (protection == Protection::ESP) {
                EspMode espMode = parseEnumElem(espModeEnum, spdEntryElem, "EspMode", defaultEspMode);
                spdEntry->setEspMode(espMode);

                EncryptionAlg encryptionAlg = parseEnumElem(encryptionAlgEnum, spdEntryElem, "EncryptionAlg", defaultEncryptionAlg, EncryptionAlg::NONE);
                AuthenticationAlg authenticationAlg = parseEnumElem(authenticationAlgEnum, spdEntryElem, "AuthenticationAlg", defaultAuthenticationAlg, AuthenticationAlg::NONE);
                spdEntry->setEnryptionAlg(encryptionAlg);
                spdEntry->setAuthenticationAlg(authenticationAlg);

                spdEntry->setMaxTfcPadLength(getParameterIntValue(spdEntryElem, "MaxTfcPadLength", defaultMaxTfcPadLength));

                if ((espMode == EspMode::CONFIDENTIALITY || espMode == EspMode::COMBINED) && encryptionAlg == EncryptionAlg::NONE)
                    throw cRuntimeError("Cannot set encryptionAlg=NONE if confidentiality was requested in espMode, at %s", spdEntryElem->getSourceLocation());

                if (espMode == EspMode::INTEGRITY && authenticationAlg == AuthenticationAlg::NONE)
                    throw cRuntimeError("Cannot set authenticationAlg=NONE if espMode=INTEGRITY is selected, at %s", spdEntryElem->getSourceLocation());

                if (espMode == EspMode::COMBINED && authenticationAlg == AuthenticationAlg::NONE && getIntegrityCheckValueBitLength(encryptionAlg) == 0)
                    throw cRuntimeError("No authenticationAlg set and EncryptionAlg %s does not provide authentication, required by EspMode=COMBINED, at %s",
                            encryptionAlgEnum.nameOf(encryptionAlg), getUniqueChild(spdEntryElem, "EncryptionAlg")->getSourceLocation());
            }

            if (protection == Protection::AH) {
                AuthenticationAlg authenticationAlg = parseEnumElem(authenticationAlgEnum, spdEntryElem, "AuthenticationAlg");
                if (authenticationAlg == AuthenticationAlg::NONE)
                    throw cRuntimeError("Cannot set authenticationAlg=NONE for AH protection, at %s", spdEntryElem->getSourceLocation());
                spdEntry->setAuthenticationAlg(authenticationAlg);
            }

            // load SA details
            for (cXMLElement *saEntryElem : spdEntryElem->getChildrenByTagName("SecurityAssociation")) {
                checkTags(saEntryElem, "SPI Selector");

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
    checkTags(selectorElem, "LocalAddress RemoteAddress Protocol LocalPort RemotePort ICMPType ICMPCode");

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

inline const char *nulltoempty(const char *s) {return s ? s : nullptr;}

template<typename E>
E IPsec::parseEnumElem(const Enum<E>& enum_, const cXMLElement *parentElem, const char *childElemName, E defaultValue, E defaultValue2)
{
    const cXMLElement *elem = getUniqueChildIfExists(parentElem, childElemName);
    if (elem == nullptr) {
        if (defaultValue != (E)-1)
            return defaultValue;
        else if (defaultValue2 != (E)-1)
            return defaultValue2;
        else
            throw cRuntimeError("Missing <%s> child in <%s> element at %s, and no default value given as module parameter",
                    childElemName, parentElem->getTagName(), parentElem->getSourceLocation());
    }
    try {
        return enum_.valueFor(nulltoempty(elem->getNodeValue()));
    }
    catch (std::exception& e) {
        throw cRuntimeError("%s at %s", e.what(), elem->getSourceLocation());
    }
}

void IPsec::initialize(int stage)
{
    if (stage == INITSTAGE_NETWORK_LAYER_3) {
        ahProtectOutDelay = &par("ahProtectOutDelay");
        ahProtectInDelay = &par("ahProtectInDelay");

        espProtectOutDelay = &par("espProtectOutDelay");
        espProtectInDelay = &par("espProtectInDelay");

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

    if (ipv4datagram->getDestAddress().isMulticast()) {
        EV_INFO << "IPsec OUT BYPASS due to multicast, packet: " << egressPacketInfo.str() << std::endl;
        emit(outBypassSignal, 1L);
        outBypass++;
        return INetfilter::IHook::ACCEPT;
    }

    //search Security Policy Database
    SecurityPolicy *spdEntry = spdModule->findEntry(Direction::OUT, &egressPacketInfo);
    if (spdEntry != nullptr) {
        if (spdEntry->getAction() == Action::PROTECT) {
            emit(outProtectSignal, 1L);
            outProtect++;
            return protectDatagram(ipv4datagram, egressPacketInfo, spdEntry);
        }
        else if (spdEntry->getAction() == Action::BYPASS) {
            EV_INFO << "IPsec OUT BYPASS rule, packet: " << egressPacketInfo.str() << std::endl;
            emit(outBypassSignal, 1L);
            outBypass++;
            return INetfilter::IHook::ACCEPT;
        }
        else if (spdEntry->getAction() == Action::DISCARD) {
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

    bool haveEncryption = sadEntry->getEspMode()==EspMode::CONFIDENTIALITY || sadEntry->getEspMode()==EspMode::COMBINED;
    ASSERT(haveEncryption == (sadEntry->getEnryptionAlg()!=EncryptionAlg::NONE));

    // Handle encryption part. Compute padding according to RFC 4303 Sections 2.4 and 2.5
    unsigned int tcfPadding = intuniform(0, sadEntry->getMaxTfcPadLength());
    unsigned int paddableLength = transport->getByteLength() + tcfPadding + ESP_FIXED_PAYLOAD_TRAILER_BYTES;
    unsigned int blockSize = getBlockSizeBytes(sadEntry->getEnryptionAlg());
    unsigned int paddedLength = blockSize * ((paddableLength + blockSize - 1) / blockSize);
    paddedLength = (paddedLength+3) & ~3;  // multiple of 32 bits
    unsigned int padLength = paddedLength - paddableLength;
    ASSERT(padLength <= 255);  // otherwise it won't fit into the 8-bit field in the ESP packet

    // Compute Initialization Vector and Integrity Check Value lengths if necessary
    unsigned int icvBytes;
    unsigned int ivBytes;
    switch (sadEntry->getEspMode()) {
        case EspMode::NONE:
            ASSERT(false); // forbidden by RFC 4301
            break;
        case EspMode::INTEGRITY:
            ivBytes = getInitializationVectorBitLength(sadEntry->getAuthenticationAlg()) / 8;
            icvBytes = getIntegrityCheckValueBitLength(sadEntry->getAuthenticationAlg()) / 8;
            ASSERT2(icvBytes != 0, "ICV must not be empty if integrity is requested for the SA");
            break;
        case EspMode::CONFIDENTIALITY:
            ivBytes = getInitializationVectorBitLength(sadEntry->getEnryptionAlg()) / 8;
            icvBytes = 0; // no checksum
            break;
        case EspMode::COMBINED: {
            ivBytes = getInitializationVectorBitLength(sadEntry->getEnryptionAlg()) / 8;
            bool haveSeparateAuthenticationAlg = sadEntry->getAuthenticationAlg() != AuthenticationAlg::NONE;
            if (haveSeparateAuthenticationAlg)
                icvBytes = getIntegrityCheckValueBitLength(sadEntry->getAuthenticationAlg()) / 8;
            else
                icvBytes = getIntegrityCheckValueBitLength(sadEntry->getEnryptionAlg()) / 8; // assume encryption alg's supports combined mode (contains hash function)
            ASSERT2(icvBytes != 0, "ICV must not be empty if integrity is requested for the SA");
            break;
        }
    }

    // compute total length
    unsigned int len = ESP_FIXED_HEADER_BYTES + ivBytes + paddedLength + icvBytes;

    // create ESP packet
    espPacket->setPadLength(padLength);
    espPacket->setSequenceNumber(sadEntry->getAndIncSeqNum());
    espPacket->setSpi(sadEntry->getSpi());
    espPacket->setNextHeader(transportType);
    espPacket->encapsulate(transport);
    espPacket->setByteLength(len);

    return espPacket;
}

cPacket *IPsec::ahProtect(cPacket *transport, SecurityAssociation *sadEntry, int transportType)
{
    IPsecAuthenticationHeader *ahPacket = new IPsecAuthenticationHeader();

    unsigned int icvBytes = getIntegrityCheckValueBitLength(sadEntry->getAuthenticationAlg()) / 8;
    unsigned int ivBytes  = getInitializationVectorBitLength(sadEntry->getAuthenticationAlg()) / 8;

    unsigned int len = IP_AH_HEADER_BYTES + ivBytes + icvBytes;
    ahPacket->setByteLength(len);

    ahPacket->setSequenceNumber(sadEntry->getAndIncSeqNum());
    ahPacket->setSpi(sadEntry->getSpi());
    ahPacket->encapsulate(transport);
    ahPacket->setNextHeader(transportType);

    return ahPacket;
}

int IPsec::getIntegrityCheckValueBitLength(EncryptionAlg alg)
{
    switch (alg) {
        // Confidentiality only
        case EncryptionAlg::NONE: return 0;
        case EncryptionAlg::DES: return 0;
        case EncryptionAlg::TRIPLE_DES: return 0;

        case EncryptionAlg::AES_CBC_128:
        case EncryptionAlg::AES_CBC_192:
        case EncryptionAlg::AES_CBC_256: return 0;

        // Combined mode
        case EncryptionAlg::AES_CCM_8_128:
        case EncryptionAlg::AES_CCM_8_192:
        case EncryptionAlg::AES_CCM_8_256: return 64;

        case EncryptionAlg::AES_CCM_16_128:
        case EncryptionAlg::AES_CCM_16_192:
        case EncryptionAlg::AES_CCM_16_256: return 128;

        case EncryptionAlg::AES_GCM_16_128:
        case EncryptionAlg::AES_GCM_16_192:
        case EncryptionAlg::AES_GCM_16_256: return 128;

        case EncryptionAlg::CHACHA20_POLY1305: return 128;
    }
    ASSERT(false);
    return 0;
}

int IPsec::getInitializationVectorBitLength(EncryptionAlg alg)
{
    switch (alg) {
        case EncryptionAlg::NONE: return 0;
        // DES RFC 2405
        case EncryptionAlg::DES: return 64;
        // 3DES RFC 2451
        case EncryptionAlg::TRIPLE_DES: return 64;
        // AES_CBC RFC 3602
        case EncryptionAlg::AES_CBC_128:
        case EncryptionAlg::AES_CBC_192:
        case EncryptionAlg::AES_CBC_256: return 128;
        // AES_CCM RFC 4309
        case EncryptionAlg::AES_CCM_8_128:
        case EncryptionAlg::AES_CCM_8_192:
        case EncryptionAlg::AES_CCM_8_256:
        case EncryptionAlg::AES_CCM_16_128:
        case EncryptionAlg::AES_CCM_16_192:
        case EncryptionAlg::AES_CCM_16_256: return 64;
        // AES_GCM RFC 4106
        case EncryptionAlg::AES_GCM_16_128:
        case EncryptionAlg::AES_GCM_16_192:
        case EncryptionAlg::AES_GCM_16_256: return 64;
        // CHACHA20_POLY1305 RFC 7634
        case EncryptionAlg::CHACHA20_POLY1305: return 64;
    }

    ASSERT(false);
    return 0;
}

int IPsec::getBlockSizeBytes(EncryptionAlg alg)
{
    switch (alg) {
        case EncryptionAlg::NONE: return 1; // the "NULL" cipher is essentially a do-nothing cipher with blocksize=1
        // DES RFC 2405
        case EncryptionAlg::DES: return 64;
        // 3DES RFC 2451
        case EncryptionAlg::TRIPLE_DES: return 64;
        // AES_CBC RFC 3602
        case EncryptionAlg::AES_CBC_128:
        case EncryptionAlg::AES_CBC_192:
        case EncryptionAlg::AES_CBC_256: return 128;
        // AES_CCM RFC 4309
        case EncryptionAlg::AES_CCM_8_128:
        case EncryptionAlg::AES_CCM_8_192:
        case EncryptionAlg::AES_CCM_8_256:
        case EncryptionAlg::AES_CCM_16_128:
        case EncryptionAlg::AES_CCM_16_192:
        case EncryptionAlg::AES_CCM_16_256: return 128;
        // AES_GCM RFC 4106
        case EncryptionAlg::AES_GCM_16_128:
        case EncryptionAlg::AES_GCM_16_192:
        case EncryptionAlg::AES_GCM_16_256: return 128;
        // CHACHA20_POLY1305 RFC 7634
        case EncryptionAlg::CHACHA20_POLY1305: return 32; //It is a stream cipher, but when used in ESP the ciphertext needs to be aligned so that padLength and nextHeader are right aligned to multiple of 4 octets.

    }

    ASSERT(false);
    return 0;
}

int IPsec::getIntegrityCheckValueBitLength(AuthenticationAlg alg)
{
    switch (alg) {
        case AuthenticationAlg::NONE: return 0;
        case AuthenticationAlg::HMAC_MD5_96: return 96;
        case AuthenticationAlg::HMAC_SHA1: return 160;
        case AuthenticationAlg::AES_128_GMAC:
        case AuthenticationAlg::AES_192_GMAC:
        case AuthenticationAlg::AES_256_GMAC: return 128; //RFC 4543 Section 4.
        case AuthenticationAlg::HMAC_SHA2_256_128: return 128;
        case AuthenticationAlg::HMAC_SHA2_384_192: return 384;
        case AuthenticationAlg::HMAC_SHA2_512_256: return 256;
    }

    ASSERT(false);
    return 0;
}

int IPsec::getInitializationVectorBitLength(AuthenticationAlg alg)
{
    switch (alg) {
        case AuthenticationAlg::NONE: return 0;
        case AuthenticationAlg::HMAC_MD5_96: return 0;
        case AuthenticationAlg::HMAC_SHA1: return 0;
        case AuthenticationAlg::AES_128_GMAC:
        case AuthenticationAlg::AES_192_GMAC:
        case AuthenticationAlg::AES_256_GMAC: return 64; //RFC 4543 Section 4.
        case AuthenticationAlg::HMAC_SHA2_256_128: return 0;
        case AuthenticationAlg::HMAC_SHA2_384_192: return 0;
        case AuthenticationAlg::HMAC_SHA2_512_256: return 0;
    }

    ASSERT(false);
    return 0;
}

INetfilter::IHook::Result IPsec::protectDatagram(IPv4Datagram *ipv4datagram, const PacketInfo& packetInfo, SecurityPolicy *spdEntry)
{
    Enter_Method_Silent();
    cPacket *transport = ipv4datagram->decapsulate();
    double delay = 0;

    bool espProtected = false, ahProtected = false;
    for (auto saEntry : spdEntry->getEntries()) {
        if (saEntry->getRule().getSelector().matches(&packetInfo)) {
            if (saEntry->getProtection() == Protection::ESP && !espProtected && !ahProtected) { // ESP protection must precede possible AH protection
                EV_INFO << "IPsec OUT ESP PROTECT packet: " << packetInfo.str() << std::endl;

                int transportType = ipv4datagram->getTransportProtocol();
                ipv4datagram->setTransportProtocol(IP_PROT_ESP);
                transport = espProtect(transport, (saEntry), transportType);

                delay += espProtectOutDelay->doubleValue();
                espProtected = true;
            }
            else if (saEntry->getProtection() == Protection::AH && !ahProtected) {
                EV_INFO << "IPsec OUT AH PROTECT packet: " << packetInfo.str() << std::endl;

                int transportType = ipv4datagram->getTransportProtocol();
                ipv4datagram->setTransportProtocol(IP_PROT_AH);
                transport = ahProtect(transport, saEntry, transportType);

                delay += ahProtectOutDelay->doubleValue();
                ahProtected = true;
            }
            else {
                EV_WARN << "IPsec OUT PROTECT packet " << packetInfo.str() << ": matching but unused SA (repeated AH or ESP protection, or swapped AH/ESP SAs)" << std::endl;
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

    if (ipv4datagram->getDestAddress().isMulticast()) {
        EV_INFO << "IPsec IN BYPASS due to multicast, packet: " << ingressPacketInfo.str() << std::endl;
        emit(inUnprotectedBypassSignal, 1L);
        inBypass++;
        return INetfilter::IHook::ACCEPT;
    }

    double delay = 0.0;
    if (transportProtocol == IP_PROT_AH) {
        IPsecAuthenticationHeader *ah = check_and_cast<IPsecAuthenticationHeader *>(ipv4datagram->decapsulate());

        // find SA Entry in SAD
        SecurityAssociation *sadEntry = sadModule->findEntry(Direction::IN, ah->getSpi());

        if (sadEntry != nullptr) {
            if (sadEntry->getProtection() != Protection::AH) {
                EV_INFO << "IPsec IN DROP AH, SA does not prescribe AH, packet: " << ingressPacketInfo.str() << std::endl;
                emit(inProtectedDropSignal, 1L);
                inDrop++;
                return INetfilter::IHook::DROP;
            }

            // found SA
            ipv4datagram->encapsulate(ah->decapsulate());
            ipv4datagram->setTransportProtocol(ah->getNextHeader());

            delay = ahProtectInDelay->doubleValue();

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

        SecurityAssociation *sadEntry = sadModule->findEntry(Direction::IN, esp->getSpi());

        if (sadEntry != nullptr) {
            if (sadEntry->getProtection() != Protection::ESP) {
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

            delay += espProtectInDelay->doubleValue();

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

    SecurityPolicy *spdEntry = spdModule->findEntry(Direction::IN, &ingressPacketInfo);
    if (spdEntry != nullptr) {
        if (spdEntry->getAction() == Action::BYPASS) {
            EV_INFO << "IPsec BYPASS rule, packet: " << ingressPacketInfo.str() << std::endl;
            emit(inUnprotectedBypassSignal, 1L);
            inBypass++;
            return INetfilter::IHook::ACCEPT;
        }
        else if (spdEntry->getAction() == Action::DISCARD) {
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

