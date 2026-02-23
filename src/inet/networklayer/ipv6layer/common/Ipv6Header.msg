//
// Copyright (C) 2018 OpenSim Ltd.
// Copyright (C) 2005 Wei Yang, Ng
// Copyright (C) 2005 OpenSim Ltd.
// Copyright (C) 2001-2004 CTIE, Monash University
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.networklayer.common.IpProtocolId;
import inet.networklayer.contract.NetworkHeaderBase;
import inet.networklayer.contract.ipv6.Ipv6Address;

cplusplus {{
#include <iostream>
#include "inet/common/ProtocolGroup.h"
#include "inet/networklayer/contract/ipv6/Ipv6Consts.h"
}}

namespace inet;

cplusplus {{
class Ipv6ExtensionHeader;
std::ostream& operator<<(std::ostream& os, Ipv6ExtensionHeader eh);
}}

//
// This serves as the base class for all the Ipv6 extension headers.
//
class Ipv6ExtensionHeader extends cObject
{
    @packetData;
    uint8_t extensionType;
    B byteLength = B(0);   // byteLength = n * 8;
}

cplusplus(Ipv6ExtensionHeader) {{
  public:
    int getOrder() const;
}}
//
// Ipv6 datagram. RFC 2460 Section 3.
//
// Header fields not explicitly modelled:
//    - payload length: will be calculated from encapsulated message length
//      and extension headers' length
//
class Ipv6Header extends NetworkHeaderBase
{
    chunkLength = IPv6_HEADER_BYTES;
    uint8_t version = 6;
    Ipv6Address srcAddress;
    Ipv6Address destAddress;
    B payloadLength = B(-1);    // The size of the payload in octets, including any extension headers. The length is set to zero when a Hop-by-Hop extension header carries a Jumbo Payload option.
    short trafficClass;    // @bits(8) Traffic Class
    // OMNeT++ 6.0:
    // short __dscp @custom @getter(getDscp) @setter(setDscp);  // @bit(6), maps to bits 0-5 of trafficClass
    // short __ecn @custom @getter(getEcn) @setter(setEcn);  // @bit(2),  maps to bits 6-7 of trafficClass

    unsigned int flowLabel;
    short hopLimit = 0;
    IpProtocolId protocolId = IP_PROT_NONE;

    Ipv6ExtensionHeader *extensionHeader[] @owned; // array of extension headers, subclassed from ~Ipv6ExtensionHeader
}

cplusplus(Ipv6Header) {{
  public:
    virtual short getDscp() const;
    virtual void setDscp(short dscp);
    virtual short getEcn() const;
    virtual void setEcn(short ecn);

    /**
     * Returns the extension header of the specified type,
     * or nullptr. If index is 0, then the first, if 1 then the
     * second extension is returned. (The datagram might
     * contain two Destination Options extension.)
     */
    virtual Ipv6ExtensionHeader *findExtensionHeaderByTypeForUpdate(IpProtocolId extensionType, int index = 0);
    virtual const Ipv6ExtensionHeader *findExtensionHeaderByType(IpProtocolId extensionType, int index = 0) const;

    /**
     * Adds an extension header to the datagram.
     * The extension headers are stored in the order specified in RFC 2460 4.1.
     */
    virtual void addExtensionHeader(Ipv6ExtensionHeader *eh);

    /**
     * Calculates the length of the Ipv6 header plus the extension
     * headers.
     */
    virtual B calculateHeaderByteLength() const;

    /**
     * Calculates the length of the unfragmentable part of Ipv6 header
     * plus the extension headers.
     */
    virtual B calculateUnfragmentableHeaderByteLength() const;

    /**
     * Calculates the length of the payload and extension headers
     * after the Fragment Header.
     */
    virtual B calculateFragmentLength() const;

    /**
     * Removes and returns the first extension header of this datagram
     */
    virtual Ipv6ExtensionHeader *removeFirstExtensionHeader();

    /**
     * Removes and returns the first extension header with the given type.
     */
    virtual Ipv6ExtensionHeader *removeExtensionHeader(IpProtocolId extensionType);

    // implements NetworkHeaderBase functions:
    virtual L3Address getSourceAddress() const override { return L3Address(getSrcAddress()); }
    virtual void setSourceAddress(const L3Address& address) override { setSrcAddress(address.toIpv6()); }
    virtual L3Address getDestinationAddress() const override { return L3Address(getDestAddress()); }
    virtual void setDestinationAddress(const L3Address& address) override { setDestAddress(address.toIpv6()); }
    virtual const Protocol *getProtocol() const override { return ProtocolGroup::getIpProtocolGroup()->findProtocol(getProtocolId()); }
    virtual void setProtocol(const Protocol *protocol) override { setProtocolId(static_cast<IpProtocolId>(ProtocolGroup::getIpProtocolGroup()->getProtocolNumber(protocol))); }
    virtual bool isFragment() const override { return findExtensionHeaderByType(IP_PROT_IPv6EXT_FRAGMENT) != nullptr; }
}}
