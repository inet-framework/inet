//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ACKINGMACHEADERSERIALIZER_H
#define __INET_ACKINGMACHEADERSERIALIZER_H

#include "inet/common/packet/recorder/PcapRecorder.h"
#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/linklayer/acking/AckingMacHeader_m.h"

namespace inet {

/**
 * Converts between AckingMacHeader and binary network byte order mac header.
 */
class INET_API AckingMacHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    AckingMacHeaderSerializer() : FieldsChunkSerializer() {}
};

class INET_API AckingMacToEthernetPcapRecorderHelper : public cObject, public PcapRecorder::IHelper
{
    virtual PcapLinkType protocolToLinkType(const Protocol *protocol) const;
    virtual bool matchesLinkType(PcapLinkType pcapLinkType, const Protocol *protocol) const;
    virtual Packet *tryConvertToLinkType(const Packet *packet, PcapLinkType pcapLinkType, const Protocol *protocol) const;
};

} // namespace inet

#endif

