//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPHEADERSERIALIZER_H
#define __INET_TCPHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {

namespace tcp {

/**
 * Converts between TcpHeader and binary (network byte order) Tcp header.
 */
class INET_API TcpHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serializeOption(MemoryOutputStream& stream, const TcpOption *option) const;
    virtual TcpOption *deserializeOption(MemoryInputStream& stream) const;

    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    TcpHeaderSerializer() : FieldsChunkSerializer() {}
};

} // namespace tcp

} // namespace inet

#endif

