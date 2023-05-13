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

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/XMLUtils.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/ipv4/IcmpHeader.h"
#include "inet/networklayer/ipv4/ipsec/IPsecAuthenticationHeader_m.h"
#include "inet/networklayer/ipv4/ipsec/IPsecEncapsulatingSecurityPayload_m.h"

#ifdef INET_WITH_TCP_COMMON
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#endif
#ifdef INET_WITH_UDP
#include "inet/transportlayer/udp/UdpHeader_m.h"
#endif

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

    auto addrConv = [](std::string s) {return L3AddressResolver().resolve(s.c_str(), L3AddressResolver::ADDR_IPv4).toIpv4();};
    auto intConv = [](std::string s) {return atoi(s.c_str());};
    auto protocolConv = [](std::string s) {return parseProtocol(s);};
    if (const cXMLElement *localAddressElem = getUniqueChildIfExists(selectorElem, "LocalAddress"))
        selector.setLocalAddress(rangelist<Ipv4Address>::parse(localAddressElem->getNodeValue(), addrConv));
    if (const cXMLElement *remoteAddressElem = getUniqueChildIfExists(selectorElem, "RemoteAddress"))
        selector.setRemoteAddress(rangelist<Ipv4Address>::parse(remoteAddressElem->getNodeValue(), addrConv));
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
    if (stage == INITSTAGE_LOCAL) {
        ahProtectOutDelay = &par("ahProtectOutDelay");
        ahProtectInDelay = &par("ahProtectInDelay");

        espProtectOutDelay = &par("espProtectOutDelay");
        espProtectInDelay = &par("espProtectInDelay");

        ipLayer = getModuleFromPar<Ipv4>(par("networkProtocolModule"), this);
        interfaceTable.reference(this, "interfaceTableModule", true);

        //register IPsec hook
        ipLayer->registerHook(0, this);

        spdModule = getModuleFromPar<SecurityPolicyDatabase>(par("spdModule"), this);
        sadModule = getModuleFromPar<SecurityAssociationDatabase>(par("sadModule"), this);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER_PROTOCOLS) {  // TODO: was INITSTAGE_NETWORK_LAYER_3
        cXMLElement *spdConfig = par("spdConfig").xmlValue();
        initSecurityDBs(spdConfig);
    }
}

void IPsec::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        Packet *context = static_cast<Packet *>(msg->getContextPointer());
        delete msg;
        ipLayer->reinjectQueuedDatagram(context);
    }
}

INetfilter::IHook::Result IPsec::datagramPreRoutingHook(Packet *packet)
{
    // First hook at the receiver (before fragment reassembly)
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result IPsec::datagramForwardHook(Packet *packet)
{
    return INetfilter::IHook::ACCEPT;
}

PacketInfo IPsec::extractEgressPacketInfo(Packet *packet, const Ipv4Address& localAddress)
{
    PacketInfo packetInfo;
    const auto& ipv4datagram = packet->peekAtFront<Ipv4Header>();

    packetInfo.setLocalAddress(localAddress);
    packetInfo.setRemoteAddress(ipv4datagram->getDestAddress());

    packetInfo.setNextProtocol(ipv4datagram->getProtocolId());

    if (ipv4datagram->getProtocolId() == IP_PROT_TCP) {
        const auto& tcpHeader = packet->peekDataAt<tcp::TcpHeader>(ipv4datagram->getChunkLength());
        packetInfo.setLocalPort(tcpHeader->getSourcePort());
        packetInfo.setRemotePort(tcpHeader->getDestinationPort());
    }
    else if (ipv4datagram->getProtocolId() == IP_PROT_UDP) {
        const auto& udpHeader = packet->peekDataAt<UdpHeader>(ipv4datagram->getChunkLength());
        packetInfo.setLocalPort(udpHeader->getSourcePort());
        packetInfo.setRemotePort(udpHeader->getDestinationPort());
    }
    else if (ipv4datagram->getProtocolId() == IP_PROT_ICMP) {
        const auto& icmpHeader = packet->peekDataAt<IcmpHeader>(ipv4datagram->getChunkLength());
        packetInfo.setIcmpType(icmpHeader->getType());
        packetInfo.setIcmpCode(icmpHeader->getCode());
    }
    return packetInfo;
}

INetfilter::IHook::Result IPsec::datagramPostRoutingHook(Packet *packet)
{
    auto outIE = interfaceTable->getInterfaceById(packet->getTag<InterfaceReq>()->getInterfaceId());
    return processEgressPacket(packet, outIE->getIpv4Address());
}

INetfilter::IHook::Result IPsec::processEgressPacket(Packet *packet, const Ipv4Address& localAddress)
{
    // packet will be fragmented (if necessary) later

    PacketInfo egressPacketInfo = extractEgressPacketInfo(packet, localAddress);

    const auto& ipv4datagram = packet->peekAtFront<Ipv4Header>();

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
            return protectDatagram(packet, egressPacketInfo, spdEntry);
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

PacketInfo IPsec::extractIngressPacketInfo(Packet *packet)
{
    PacketInfo packetInfo;

    const auto& ipv4datagram = packet->peekAtFront<Ipv4Header>();
    packetInfo.setLocalAddress(ipv4datagram->getDestAddress());
    packetInfo.setRemoteAddress(ipv4datagram->getSrcAddress());

    packetInfo.setNextProtocol(ipv4datagram->getProtocolId());

    if (ipv4datagram->getProtocolId() == IP_PROT_TCP) {
        const auto& tcpHeader = packet->peekDataAt<tcp::TcpHeader>(ipv4datagram->getChunkLength());
        packetInfo.setLocalPort(tcpHeader->getDestinationPort());
        packetInfo.setRemotePort(tcpHeader->getSourcePort());
    }
    else if (ipv4datagram->getProtocolId() == IP_PROT_UDP) {
        const auto& udpHeader = packet->peekDataAt<UdpHeader>(ipv4datagram->getChunkLength());
        packetInfo.setLocalPort(udpHeader->getDestinationPort());
        packetInfo.setRemotePort(udpHeader->getSourcePort());
    }
    else if (ipv4datagram->getProtocolId() == IP_PROT_ICMP) {
        const auto& icmpHeader = packet->peekDataAt<IcmpHeader>(ipv4datagram->getChunkLength());
        packetInfo.setIcmpType(icmpHeader->getType());
        packetInfo.setIcmpCode(icmpHeader->getCode());
    }
    return packetInfo;
}

void IPsec::espProtect(Packet *transport, SecurityAssociation *sadEntry, int transportType)
{
    bool haveEncryption = sadEntry->getEspMode()==EspMode::CONFIDENTIALITY || sadEntry->getEspMode()==EspMode::COMBINED;
    ASSERT(haveEncryption == (sadEntry->getEnryptionAlg() != EncryptionAlg::NONE));

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

    // encrypting:
    if (tcfPadding > 0)
        transport->insertAtBack(makeShared<ByteCountChunk>(B(tcfPadding)));
    if (padLength > 0)
        transport->insertAtBack(makeShared<ByteCountChunk>(B(padLength)));
    auto data = transport->removeData();
    data->markImmutable();
    auto encryptedData = makeShared<EncryptedChunk>(data, data->getChunkLength() + B(ivBytes));
    transport->insertData(encryptedData);

    // create ESP packet
    const auto& espHeader = makeShared<IPsecEspHeader>();
    espHeader->setSequenceNumber(sadEntry->getAndIncSeqNum());
    espHeader->setSpi(sadEntry->getSpi());
    transport->insertAtFront(espHeader);

    const auto& espTrailer = makeShared<IPsecEspTrailer>();
    espTrailer->setPadLength(padLength);
    espTrailer->setNextHeader(transportType);
    transport->insertAtBack(espTrailer);

    // Add integrity check value if needed
    if (icvBytes)
        transport->insertAtBack(makeShared<ByteCountChunk>(B(icvBytes)));

    ASSERT(transport->getByteLength() == len);
}

void IPsec::ahProtect(Packet *transport, SecurityAssociation *sadEntry, int transportType)
{
    unsigned int icvBytes = getIntegrityCheckValueBitLength(sadEntry->getAuthenticationAlg()) / 8;
    unsigned int ivBytes  = getInitializationVectorBitLength(sadEntry->getAuthenticationAlg()) / 8;

    unsigned int len = IP_AH_HEADER_BYTES + ivBytes + icvBytes;

    // encrypting:
    auto data = transport->removeData();
    data->markImmutable();
    auto encryptedData = makeShared<EncryptedChunk>(data, data->getChunkLength() + B(ivBytes));
    transport->insertData(encryptedData);

    const auto& ahHeader = makeShared<IPsecAuthenticationHeader>();
    ahHeader->setSequenceNumber(sadEntry->getAndIncSeqNum());
    ahHeader->setSpi(sadEntry->getSpi());
    ahHeader->setNextHeader(transportType);
    transport->insertAtFront(ahHeader);

    // Add integrity check value if needed
    if (icvBytes)
        transport->insertAtBack(makeShared<ByteCountChunk>(B(icvBytes)));

    ASSERT(transport->getByteLength() == len);
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

INetfilter::IHook::Result IPsec::protectDatagram(Packet *packet, const PacketInfo& packetInfo, SecurityPolicy *spdEntry)
{
    Enter_Method_Silent();

    auto ipv4Header = packet->removeAtFront<Ipv4Header>();

    auto padding = packet->getDataLength() + ipv4Header->getChunkLength() - ipv4Header->getTotalLengthField();
    if (padding > b(0))
        packet->removeAtBack(padding);

    double delay = 0;

    bool espProtected = false, ahProtected = false;
    for (auto saEntry : spdEntry->getEntries()) {
        if (saEntry->getRule().getSelector().matches(&packetInfo)) {
            if (saEntry->getProtection() == Protection::ESP && !espProtected && !ahProtected) { // ESP protection must precede possible AH protection
                EV_INFO << "IPsec OUT ESP PROTECT packet: " << packetInfo.str() << std::endl;

                int transportType = ipv4Header->getProtocolId();
                espProtect(packet, (saEntry), transportType);
                ipv4Header->setProtocolId(IP_PROT_ESP);

                delay += espProtectOutDelay->doubleValue();
                espProtected = true;
            }
            else if (saEntry->getProtection() == Protection::AH && !ahProtected) {
                EV_INFO << "IPsec OUT AH PROTECT packet: " << packetInfo.str() << std::endl;

                int transportType = ipv4Header->getProtocolId();
                ipv4Header->setProtocolId(IP_PROT_AH);
                ahProtect(packet, saEntry, transportType);

                delay += ahProtectOutDelay->doubleValue();
                ahProtected = true;
            }
            else {
                EV_WARN << "IPsec OUT PROTECT packet " << packetInfo.str() << ": matching but unused SA (repeated AH or ESP protection, or swapped AH/ESP SAs)" << std::endl;
            }
        }
    }

    ipv4Header->setTotalLengthField(ipv4Header->getChunkLength() + packet->getDataLength());
    packet->insertAtFront(ipv4Header);

    if (delay > 0 || lastProtectedOut > simTime()) {
        cMessage *selfmsg = new cMessage("IPsecProtectOutDelay");
        selfmsg->setContextPointer(packet);
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

INetfilter::IHook::Result IPsec::datagramLocalInHook(Packet *packet)
{
    return processIngressPacket(packet);
}

INetfilter::IHook::Result IPsec::processIngressPacket(Packet *packet)
{
    Enter_Method_Silent();

    PacketInfo ingressPacketInfo = extractIngressPacketInfo(packet);

    int transportProtocol = ingressPacketInfo.getNextProtocol();

    if (ingressPacketInfo.getLocalAddress().isMulticast()) {
        EV_INFO << "IPsec IN BYPASS due to multicast, packet: " << ingressPacketInfo.str() << std::endl;
        emit(inUnprotectedBypassSignal, 1L);
        inBypass++;
        return INetfilter::IHook::ACCEPT;
    }

    double delay = 0.0;
    if (transportProtocol == IP_PROT_AH) {
        auto ipv4Header = packet->removeAtFront<Ipv4Header>();

        auto ah = packet->removeAtFront<IPsecAuthenticationHeader>();

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
            auto ahHeader = packet->removeAtFront<IPsecAuthenticationHeader>();

            // decrypting:
            auto encryptedData = packet->removeAtFront<EncryptedChunk>();
            packet->removeData(Chunk::PF_ALLOW_EMPTY);
            auto data = encryptedData->getChunk();
            packet->insertData(data);
            ipv4Header->setProtocolId((IpProtocolId)ahHeader->getNextHeader());
            ipv4Header->setTotalLengthField(ipv4Header->getChunkLength() + data->getChunkLength());
            packet->insertAtFront(ipv4Header);

            // TODO check icv, ...

            delay = ahProtectInDelay->doubleValue();

            if (ahHeader->getNextHeader() != IP_PROT_ESP) {
                emit(inProtectedAcceptSignal, 1L);
                inAccept++;

                if (delay > 0 || lastProtectedIn > simTime()) {
                    cMessage *selfmsg = new cMessage("IPsecProtectInDelay");
                    selfmsg->setContextPointer(packet);
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

        if (sadEntry == nullptr) {
            EV_INFO << "IPsec IN DROP AH, no matching rule, packet: " << ingressPacketInfo.str() << std::endl;
            emit(inProtectedDropSignal, 1L);
            inDrop++;
            return INetfilter::IHook::DROP;
        }
    }

    if (transportProtocol == IP_PROT_ESP) {
        auto ipv4Header = packet->removeAtFront<Ipv4Header>();
        auto espHeader = packet->removeAtFront<IPsecEspHeader>();

        SecurityAssociation *sadEntry = sadModule->findEntry(Direction::IN, espHeader->getSpi());

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

            // decrypting:
            auto encryptedData = packet->removeAtFront<EncryptedChunk>();
            auto espTrailer = packet->removeAtFront<IPsecEspTrailer>();
            packet->removeData(Chunk::PF_ALLOW_EMPTY); // remove icv bytes
            auto data = encryptedData->getChunk();
            packet->insertData(data);
            if (espTrailer->getPadLength() > 0)
                packet->removeAtBack(B(espTrailer->getPadLength()));
            ipv4Header->setProtocolId((IpProtocolId)espTrailer->getNextHeader());
            ipv4Header->setTotalLengthField(ipv4Header->getChunkLength() + data->getChunkLength() - B(espTrailer->getPadLength()));
            packet->insertAtFront(ipv4Header);

            delay += espProtectInDelay->doubleValue();

            if (delay > 0 || lastProtectedIn > simTime()) {
                cMessage *selfmsg = new cMessage("IPsecProtectInDelay");
                selfmsg->setContextPointer(packet);
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

INetfilter::IHook::Result IPsec::datagramLocalOutHook(Packet *packet)
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

