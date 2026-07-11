//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/pcep/PcepMessagesSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/networklayer/pcep/PcepCommon.h"

namespace inet {

// Also registering the base PcepMessage (in addition to each concrete subtype
// below) so that generic code paths popping the base type off a byte stream (e.g.
// Pce::socketDataArrived's/Pcc::socketDataArrived's queue->pop<PcepMessage>())
// can dispatch too -- mirrors LdpPacketSerializer's registration pattern.
Register_Serializer(PcepMessage, PcepMessagesSerializer);
Register_Serializer(PcepOpen, PcepMessagesSerializer);
Register_Serializer(PcepKeepalive, PcepMessagesSerializer);

// RFC 5440 Section 7.2/7.3: OPEN object identity (Object-Class=1, Object-Type=1)
static const uint8_t PCEP_OBJ_CLASS_OPEN = 1;
static const uint8_t PCEP_OBJ_TYPE_OPEN = 1;

void PcepMessagesSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& pcepMsg = staticPtrCast<const PcepMessage>(chunk);

    // RFC 5440 Section 6.1: Message-Length is INCLUSIVE of the 4-byte Common Header itself.
    uint16_t messageLength = pcepMsg->getChunkLength().get<B>();

    stream.writeNBitsOfUint64Be(pcepMsg->getVersion(), 3);
    stream.writeNBitsOfUint64Be(0, 5); // Flags: unused (RFC 5440 Section 6.1)
    stream.writeByte((uint8_t)pcepMsg->getType());
    stream.writeUint16Be(messageLength);

    switch (pcepMsg->getType()) {
        case PCEP_OPEN: {
            const auto& open = staticPtrCast<const PcepOpen>(pcepMsg);
            // Common object header (RFC 5440 Section 7.2); Object-Length is INCLUSIVE of this 4-byte header.
            stream.writeByte(PCEP_OBJ_CLASS_OPEN);
            stream.writeNBitsOfUint64Be(PCEP_OBJ_TYPE_OPEN, 4);
            stream.writeNBitsOfUint64Be(0, 2); // reserved
            stream.writeBit(false); // P (Processing-Rule) flag: not modeled
            stream.writeBit(false); // I (Ignore) flag: not modeled
            stream.writeUint16Be(PCEP_OPEN_OBJECT_BYTES.get<B>());
            // OPEN object body (RFC 5440 Section 7.3)
            stream.writeNBitsOfUint64Be(1, 3); // PCEP version (always 1)
            stream.writeNBitsOfUint64Be(0, 5); // Flags: unused
            stream.writeByte(open->getKeepaliveTime());
            stream.writeByte(open->getDeadTimer());
            stream.writeByte(open->getSid());
            break;
        }
        case PCEP_KEEPALIVE:
            // no object (RFC 5440 Section 6.4)
            break;
        default:
            throw cRuntimeError("PcepMessagesSerializer: cannot serialize unknown PCEP message type %d", pcepMsg->getType());
    }
}

const Ptr<Chunk> PcepMessagesSerializer::deserialize(MemoryInputStream& stream) const
{
    uint8_t version = stream.readNBitsToUint64Be(3);
    stream.readNBitsToUint64Be(5); // Flags: unused
    int type = stream.readByte();
    // Message-Length (RFC 5440 Section 6.1): INCLUSIVE of the 4-byte Common Header
    // itself -- unlike LdpPacketSerializer's exclusive PDU-length convention, this
    // needs no offset arithmetic to become the chunk length below.
    uint16_t messageLength = stream.readUint16Be();

    Ptr<PcepMessage> pcepMsg;
    switch (type) {
        case PCEP_OPEN: {
            auto open = makeShared<PcepOpen>();
            stream.readByte(); // Object-Class, assumed PCEP_OBJ_CLASS_OPEN
            stream.readNBitsToUint64Be(4); // Object-Type, assumed PCEP_OBJ_TYPE_OPEN
            stream.readNBitsToUint64Be(2); // reserved
            stream.readBit(); // P flag: not modeled
            stream.readBit(); // I flag: not modeled
            stream.readUint16Be(); // Object-Length, assumed PCEP_OPEN_OBJECT_BYTES
            stream.readNBitsToUint64Be(3); // PCEP version, redundant with the Common Header's own version field
            stream.readNBitsToUint64Be(5); // Flags: unused
            open->setKeepaliveTime(stream.readByte());
            open->setDeadTimer(stream.readByte());
            open->setSid(stream.readByte());
            pcepMsg = open;
            break;
        }
        case PCEP_KEEPALIVE: {
            // no message parameters (RFC 5440 Section 6.4)
            pcepMsg = makeShared<PcepKeepalive>();
            break;
        }
        default: {
            auto unknown = makeShared<PcepMessage>();
            unknown->markIncorrect();
            pcepMsg = unknown;
            break;
        }
    }

    pcepMsg->setVersion(version);
    pcepMsg->setType(type);
    pcepMsg->setChunkLength(B(messageLength));
    return pcepMsg;
}

} // namespace inet
