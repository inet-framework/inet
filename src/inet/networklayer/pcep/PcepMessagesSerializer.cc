//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/pcep/PcepMessagesSerializer.h"

#include <cstring>

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
Register_Serializer(PcepPcreq, PcepMessagesSerializer);
Register_Serializer(PcepPcrep, PcepMessagesSerializer);

// RFC 5440 Section 7.2/7.3: OPEN object identity (Object-Class=1, Object-Type=1)
static const uint8_t PCEP_OBJ_CLASS_OPEN = 1;
static const uint8_t PCEP_OBJ_TYPE_OPEN = 1;

// RFC 5440 Section 7 object identities used by PCReq/PCRep (Workstream F4 Phase 2)
static const uint8_t PCEP_OBJ_CLASS_NOPATH = 3;
static const uint8_t PCEP_OBJ_TYPE_NOPATH = 1;
static const uint8_t PCEP_OBJ_CLASS_ENDPOINTS = 4;
static const uint8_t PCEP_OBJ_TYPE_ENDPOINTS_IPV4 = 1;
static const uint8_t PCEP_OBJ_CLASS_BANDWIDTH = 5;
static const uint8_t PCEP_OBJ_TYPE_BANDWIDTH_REQUESTED = 1;
static const uint8_t PCEP_OBJ_CLASS_ERO = 7;
static const uint8_t PCEP_OBJ_TYPE_ERO_IPV4 = 1; // reuses RFC 3209 Section 4.3.3.1's IPv4 prefix subobject format verbatim
static const uint8_t PCEP_OBJ_CLASS_LSPA = 9;
static const uint8_t PCEP_OBJ_TYPE_LSPA = 1;
static const uint8_t PCEP_OBJ_CLASS_RP = 2;
static const uint8_t PCEP_OBJ_TYPE_RP = 1;

namespace {

// RFC 5440 Section 7.2: common object header -- Object-Class(8 bits) +
// Object-Type(4 bits)/Res(2 bits)/P(1 bit)/I(1 bit) + Object-Length(16 bits,
// INCLUSIVE of this 4-byte header) -- the same layout the OPEN object already
// hand-writes inline below; factored out here since PCReq/PCRep carry several
// objects each.
void writePcepObjectHeader(MemoryOutputStream& stream, uint8_t objClass, uint8_t objType, uint16_t length)
{
    stream.writeByte(objClass);
    stream.writeNBitsOfUint64Be(objType, 4);
    stream.writeNBitsOfUint64Be(0, 2); // reserved
    stream.writeBit(false); // P (Processing-Rule) flag: not modeled
    stream.writeBit(false); // I (Ignore) flag: not modeled
    stream.writeUint16Be(length);
}

// Consumes a common object header, returning its Object-Length (needed to size a
// variable-length object's body, e.g. the ERO below); Object-Class/Object-Type are
// assumed correct from context (mirrors the OPEN object's own deserialize()
// "assumed" comments) rather than validated.
uint16_t readPcepObjectHeader(MemoryInputStream& stream)
{
    stream.readByte(); // Object-Class, assumed by context
    stream.readNBitsToUint64Be(4); // Object-Type, assumed by context
    stream.readNBitsToUint64Be(2); // reserved
    stream.readBit(); // P flag: not modeled
    stream.readBit(); // I flag: not modeled
    return stream.readUint16Be(); // Object-Length
}

// Peeks the next object's Class-Num (first byte of its header) without consuming
// it -- used to tell a PCRep's ERO object from its NO-PATH object apart, neither of
// which has any other on-wire presence indicator (mirrors RsvpTeSerializer's own
// peekClassNum() helper, duplicated here rather than shared to keep the pcep and
// rsvpte serializers decoupled).
uint8_t peekPcepObjectClass(MemoryInputStream& stream)
{
    B pos = stream.getPosition();
    uint8_t objClass = stream.readByte();
    stream.seek(pos);
    return objClass;
}

// C++17 has no std::bit_cast (that's C++20); a memcpy-based reinterpretation is
// the standard portable way to punch IEEE-754 float bits into/out of a uint32_t
// without violating strict aliasing (mirrors RsvpTeSerializer's identical helper,
// duplicated here for the same decoupling reason as peekPcepObjectClass above).
template<typename To, typename From>
To bitCast(From from)
{
    static_assert(sizeof(To) == sizeof(From), "bitCast: size mismatch");
    To to;
    std::memcpy(&to, &from, sizeof(To));
    return to;
}

// RFC 5440 Section 7.9: PCEP's ERO object reuses RFC 3209 Section 4.3.3.1's IPv4
// prefix subobject format verbatim -- L bit + Type(7 bits, =1 IPv4 prefix),
// Length(1, =8), Address(4), Prefix Length(1, fixed at 32: this model's EroObj
// carries no prefix length, only single router addresses), Reserved(1) -- 8
// bytes/hop. Mirrors ~RsvpTeSerializer::serializeEro/deserializeEro's identical
// on-wire encoding (kept as a separate, local implementation to avoid coupling
// the pcep and rsvpte serializers).
void writePcepEro(MemoryOutputStream& stream, const EroVector& ero)
{
    writePcepObjectHeader(stream, PCEP_OBJ_CLASS_ERO, PCEP_OBJ_TYPE_ERO_IPV4, static_cast<uint16_t>(4 + 8 * ero.size()));
    for (const auto& hop : ero) {
        stream.writeByte((hop.L ? 0x80 : 0x00) | 0x01); // L bit + Type=1 (IPv4 prefix)
        stream.writeByte(8); // subobject length
        stream.writeIpv4Address(hop.node);
        stream.writeByte(32); // prefix length
        stream.writeByte(0); // reserved
    }
}

EroVector readPcepEro(MemoryInputStream& stream)
{
    uint16_t length = readPcepObjectHeader(stream);
    int n = (length - 4) / 8;
    EroVector ero;
    for (int i = 0; i < n; i++) {
        uint8_t typeByte = stream.readByte();
        stream.readByte(); // subobject length, assumed 8
        EroObj hop;
        hop.L = (typeByte & 0x80) != 0;
        hop.node = stream.readIpv4Address();
        stream.readByte(); // prefix length, assumed 32
        stream.readByte(); // reserved
        ero.push_back(hop);
    }
    return ero;
}

} // namespace

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
        case PCEP_PCREQ: {
            const auto& req = staticPtrCast<const PcepPcreq>(pcepMsg);
            // RP object (RFC 5440 Section 7.4)
            writePcepObjectHeader(stream, PCEP_OBJ_CLASS_RP, PCEP_OBJ_TYPE_RP, PCEP_RP_OBJECT_BYTES.get<B>());
            stream.writeUint32Be(req->getRequestId());
            // END-POINTS object (RFC 5440 Section 7.6, IPv4 C-Type)
            writePcepObjectHeader(stream, PCEP_OBJ_CLASS_ENDPOINTS, PCEP_OBJ_TYPE_ENDPOINTS_IPV4, PCEP_ENDPOINTS_OBJECT_BYTES.get<B>());
            stream.writeIpv4Address(req->getSrcAddress());
            stream.writeIpv4Address(req->getDstAddress());
            // BANDWIDTH object (RFC 5440 Section 7.7)
            writePcepObjectHeader(stream, PCEP_OBJ_CLASS_BANDWIDTH, PCEP_OBJ_TYPE_BANDWIDTH_REQUESTED, PCEP_BANDWIDTH_OBJECT_BYTES.get<B>());
            stream.writeUint32Be(bitCast<uint32_t>(static_cast<float>(req->getBandwidth())));
            // LSPA object (RFC 5440 Section 7.11)
            writePcepObjectHeader(stream, PCEP_OBJ_CLASS_LSPA, PCEP_OBJ_TYPE_LSPA, PCEP_LSPA_OBJECT_BYTES.get<B>());
            stream.writeUint32Be(req->getExcludeAny());
            stream.writeUint32Be(req->getIncludeAny());
            stream.writeByte(static_cast<uint8_t>(req->getSetupPriority()));
            stream.writeByte(0); // holding priority: not modeled
            stream.writeByte(0); // flags: not modeled
            stream.writeByte(0); // reserved
            break;
        }
        case PCEP_PCREP: {
            const auto& rep = staticPtrCast<const PcepPcrep>(pcepMsg);
            // RP object (RFC 5440 Section 7.4), echoing the request's Request-ID-number
            writePcepObjectHeader(stream, PCEP_OBJ_CLASS_RP, PCEP_OBJ_TYPE_RP, PCEP_RP_OBJECT_BYTES.get<B>());
            stream.writeUint32Be(rep->getRequestId());
            if (rep->getNoPath()) {
                // NO-PATH object (RFC 5440 Section 7.5)
                writePcepObjectHeader(stream, PCEP_OBJ_CLASS_NOPATH, PCEP_OBJ_TYPE_NOPATH, PCEP_NOPATH_OBJECT_BYTES.get<B>());
                stream.writeUint32Be(0); // Nature-of-Issue/Flags/Reserved: not modeled
            }
            else {
                // ERO object (RFC 5440 Section 7.9)
                writePcepEro(stream, rep->getEro());
            }
            break;
        }
        default:
            throw cRuntimeError("PcepMessagesSerializer: cannot serialize unknown PCEP message type %d", pcepMsg->getType());
    }
}

const Ptr<Chunk> PcepMessagesSerializer::deserialize(MemoryInputStream& stream) const
{
    B msgStart = stream.getPosition();
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
        case PCEP_PCREQ: {
            auto req = makeShared<PcepPcreq>();
            readPcepObjectHeader(stream); // RP, assumed
            req->setRequestId(stream.readUint32Be());
            readPcepObjectHeader(stream); // END-POINTS, assumed
            req->setSrcAddress(stream.readIpv4Address());
            req->setDstAddress(stream.readIpv4Address());
            readPcepObjectHeader(stream); // BANDWIDTH, assumed
            req->setBandwidth(static_cast<double>(bitCast<float>(stream.readUint32Be())));
            readPcepObjectHeader(stream); // LSPA, assumed
            req->setExcludeAny(stream.readUint32Be());
            req->setIncludeAny(stream.readUint32Be());
            req->setSetupPriority(stream.readByte());
            stream.readByte(); // holding priority: not modeled
            stream.readByte(); // flags: not modeled
            stream.readByte(); // reserved
            pcepMsg = req;
            break;
        }
        case PCEP_PCREP: {
            auto rep = makeShared<PcepPcrep>();
            readPcepObjectHeader(stream); // RP, assumed
            rep->setRequestId(stream.readUint32Be());
            B endPos = msgStart + B(messageLength);
            if (stream.getPosition() < endPos && peekPcepObjectClass(stream) == PCEP_OBJ_CLASS_NOPATH) {
                readPcepObjectHeader(stream); // NO-PATH
                stream.readUint32Be(); // Nature-of-Issue/Flags/Reserved: not modeled
                rep->setNoPath(true);
                rep->setEro(EroVector());
            }
            else {
                rep->setNoPath(false);
                rep->setEro(readPcepEro(stream));
            }
            pcepMsg = rep;
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
