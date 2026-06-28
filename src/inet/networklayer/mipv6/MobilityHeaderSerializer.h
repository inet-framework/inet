//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_MOBILITYHEADERSERIALIZER_H
#define __INET_MOBILITYHEADERSERIALIZER_H

#include "inet/common/Units.h"
#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet {

/**
 * Converts between MobilityHeader (and subclasses) and binary (network byte order)
 * MIPv6 Mobility Header as defined in RFC 6275 Section 6.1.
 *
 * Also serializes the Proxy Mobile IPv6 (RFC 5213) proxy mobility options that a
 * Binding Update / Binding Acknowledgement carries when its proxyRegistrationFlag
 * (P-flag) is set; when the flag is clear the wire format is identical to plain MIPv6.
 */
class INET_API MobilityHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    MobilityHeaderSerializer() : FieldsChunkSerializer() {}

    /**
     * Total chunk length of a Proxy Binding Update (RFC 5213) carrying the proxy
     * mobility options for the given Mobile Node Identifier length, rounded up to
     * the 8-octet Mobility Header boundary. Used by the PMIPv6 module to size the
     * PBU chunk so it round-trips through this serializer.
     */
    static B getProxyBindingUpdateLength(size_t mobileNodeIdentifierLength);

    /**
     * Like getProxyBindingUpdateLength(), but for a Proxy Binding Acknowledgement
     * (which omits the Handoff Indicator and Access Technology Type options).
     */
    static B getProxyBindingAcknowledgementLength(size_t mobileNodeIdentifierLength);
};

} // namespace inet

#endif
