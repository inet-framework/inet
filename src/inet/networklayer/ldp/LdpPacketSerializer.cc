//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ldp/LdpPacketSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

// Also registering the base LdpPacket (in addition to each concrete subtype
// below) so that generic code paths popping the base type off a byte stream
// (e.g. Ldp::socketDataArrived's queue->pop<LdpPacket>()) can dispatch too --
// mirrors the Ospfv3Packet + concrete-subtype registration pattern used by
// Ospfv3PacketSerializer.
Register_Serializer(LdpPacket, LdpPacketSerializer);
Register_Serializer(LdpHello, LdpPacketSerializer);
Register_Serializer(LdpLabelMapping, LdpPacketSerializer);
Register_Serializer(LdpLabelRequest, LdpPacketSerializer);
Register_Serializer(LdpNotify, LdpPacketSerializer);
Register_Serializer(LdpIni, LdpPacketSerializer);
Register_Serializer(LdpKeepAlive, LdpPacketSerializer);
Register_Serializer(LdpAddress, LdpPacketSerializer);

// RFC 5036 Section 3.5.3: PDU common header = Version(2) + PDU Length(2) + LSR-Id(4) + Label Space(2)
static const B LDP_PDU_HEADER_LENGTH = B(10);

// LDP TLV type codes (RFC 5036 Section 3.4)
enum LdpTlvType {
    FEC_TLV = 0x0100,
    ADDRESS_LIST_TLV = 0x0101,
    GENERIC_LABEL_TLV = 0x0200,
    STATUS_TLV = 0x0300,
    COMMON_HELLO_PARAMETERS_TLV = 0x0400,
    COMMON_SESSION_PARAMETERS_TLV = 0x0500,
};

void LdpPacketSerializer::serializeFecTlv(MemoryOutputStream& stream, const FecTlv& fec)
{
    stream.writeBit(false); // U-bit
    stream.writeNBitsOfUint64Be(FEC_TLV, 15);
    stream.writeUint16Be(8); // TLV value length: 1(elem type) + 2(addr family) + 1(prelen) + 4(prefix)
    stream.writeByte(2); // FEC Element Type = 2 (Address Prefix)
    stream.writeUint16Be(1); // Address Family = 1 (IP)
    stream.writeByte(fec.length);
    stream.writeIpv4Address(fec.addr);
}

FecTlv LdpPacketSerializer::deserializeFecTlv(MemoryInputStream& stream)
{
    stream.readBit(); // U-bit
    stream.readNBitsToUint64Be(15); // TLV type, assumed FEC_TLV (dispatch already selected this branch)
    stream.readUint16Be(); // TLV value length, assumed 8
    stream.readByte(); // FEC Element Type, assumed 2 (Address Prefix)
    stream.readUint16Be(); // Address Family, assumed 1 (IP)
    FecTlv fec;
    fec.length = stream.readByte();
    fec.addr = stream.readIpv4Address();
    return fec;
}

void LdpPacketSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& ldpPacket = staticPtrCast<const LdpPacket>(chunk);
    B totalLength = ldpPacket->getChunkLength();

    // PDU Length excludes the Version and PDU Length fields themselves (RFC 5036 3.5.3).
    uint16_t pduLength = (totalLength - B(4)).get<B>();
    // Message Length excludes the Message Type and Message Length fields themselves
    // (RFC 5036 3.6), i.e. it covers Message ID + the message's parameters.
    uint16_t messageLength = (totalLength - LDP_PDU_HEADER_LENGTH - B(4)).get<B>();

    stream.writeUint16Be(ldpPacket->getVersion());
    stream.writeUint16Be(pduLength);
    stream.writeIpv4Address(ldpPacket->getLsrId());
    stream.writeUint16Be(ldpPacket->getLabelSpace());

    stream.writeBit(false); // U-bit
    stream.writeNBitsOfUint64Be(ldpPacket->getType(), 15);
    stream.writeUint16Be(messageLength);
    stream.writeUint32Be(ldpPacket->getMessageId());

    switch (ldpPacket->getType()) {
        case HELLO: {
            const auto& hello = staticPtrCast<const LdpHello>(ldpPacket);
            // Common Hello Parameters TLV (RFC 5036 3.5.2)
            stream.writeBit(false);
            stream.writeNBitsOfUint64Be(COMMON_HELLO_PARAMETERS_TLV, 15);
            stream.writeUint16Be(4); // value length: Hold Time(2) + flags(2)
            stream.writeUint16Be(hello->getHoldTime());
            stream.writeBit(hello->getTbit());
            stream.writeBit(hello->getRbit());
            stream.writeNBitsOfUint64Be(0, 14); // reserved
            break;
        }
        case LABEL_REQUEST: {
            const auto& req = staticPtrCast<const LdpLabelRequest>(ldpPacket);
            serializeFecTlv(stream, req->getFec());
            break;
        }
        case LABEL_MAPPING:
        case LABEL_WITHDRAW:
        case LABEL_RELEASE: {
            // sendMapping() reuses the LdpLabelMapping wire fields (FEC + label) for all three types
            const auto& mapping = staticPtrCast<const LdpLabelMapping>(ldpPacket);
            serializeFecTlv(stream, mapping->getFec());
            // Generic Label TLV (RFC 5036 3.4.3)
            stream.writeBit(false);
            stream.writeNBitsOfUint64Be(GENERIC_LABEL_TLV, 15);
            stream.writeUint16Be(4); // value length: Label(4)
            stream.writeUint32Be(mapping->getLabel());
            break;
        }
        case NOTIFICATION: {
            const auto& notify = staticPtrCast<const LdpNotify>(ldpPacket);
            // Status TLV (RFC 5036 3.4.6): Status Code(4) + referenced Message ID(4) + referenced Message Type(2)
            stream.writeBit(false);
            stream.writeNBitsOfUint64Be(STATUS_TLV, 15);
            stream.writeUint16Be(10);
            stream.writeBit(false); // E-bit (fatal error indicator): not modeled, always 0
            stream.writeBit(false); // F-bit (forward indicator): not modeled, always 0
            stream.writeNBitsOfUint64Be(notify->getStatus(), 30);
            stream.writeUint32Be(0); // referenced Message ID: this model does not correlate notifications to a specific message
            stream.writeUint16Be(0); // referenced Message Type: ditto
            // sendNotify() always includes a FEC TLV alongside the Status TLV (see Ldp.h)
            serializeFecTlv(stream, notify->getFec());
            break;
        }
        case INITIALIZATION: {
            // unwired (see Ldp.ned); still fully serializable so an in-flight instance
            // never aborts a ~tND computation
            const auto& ini = staticPtrCast<const LdpIni>(ldpPacket);
            // Common Session Parameters TLV (RFC 5036 3.5.3)
            stream.writeBit(false);
            stream.writeNBitsOfUint64Be(COMMON_SESSION_PARAMETERS_TLV, 15);
            stream.writeUint16Be(16); // value length
            stream.writeUint16Be(1); // Protocol Version (always 1, per LdpPacket::version)
            stream.writeUint16Be(ini->getKeepAliveTime());
            stream.writeBit(ini->getAbit());
            stream.writeBit(ini->getDbit());
            stream.writeNBitsOfUint64Be(0, 14); // reserved
            stream.writeUint16Be(ini->getPvLim());
            stream.writeUint16Be(ini->getMaxPduLength());
            stream.writeIpv4Address(ini->getReceiverLsrId());
            stream.writeUint16Be(ini->getReceiverLabelSpace());
            break;
        }
        case KEEP_ALIVE:
            // no message parameters (RFC 5036 Section 3.5.4)
            break;
        case ADDRESS:
        case ADDRESS_WITHDRAW: {
            // unwired (see Ldp.ned); still fully serializable so an in-flight instance
            // never aborts a ~tND computation
            const auto& addr = staticPtrCast<const LdpAddress>(ldpPacket);
            size_t n = addr->getAddressesArraySize();
            // Address List TLV (RFC 5036 3.4.2)
            stream.writeBit(false);
            stream.writeNBitsOfUint64Be(ADDRESS_LIST_TLV, 15);
            stream.writeUint16Be(2 + 4 * n); // value length: Address Family(2) + n * Address(4)
            stream.writeUint16Be(addr->getAddressFamily());
            for (size_t i = 0; i < n; ++i)
                stream.writeIpv4Address(addr->getAddresses(i));
            break;
        }
        default:
            throw cRuntimeError("LdpPacketSerializer: cannot serialize unknown LDP message type %d", ldpPacket->getType());
    }
}

const Ptr<Chunk> LdpPacketSerializer::deserialize(MemoryInputStream& stream) const
{
    uint16_t version = stream.readUint16Be();
    uint16_t pduLength = stream.readUint16Be();
    Ipv4Address lsrId = stream.readIpv4Address();
    uint16_t labelSpace = stream.readUint16Be();

    stream.readBit(); // U-bit
    int type = stream.readNBitsToUint64Be(15);
    stream.readUint16Be(); // Message Length: redundant with pduLength given one-message-per-PDU; not separately validated
    uint32_t messageId = stream.readUint32Be();

    Ptr<LdpPacket> ldpPacket;
    switch (type) {
        case HELLO: {
            auto hello = makeShared<LdpHello>();
            stream.readBit(); // U-bit
            stream.readNBitsToUint64Be(15); // TLV type, assumed COMMON_HELLO_PARAMETERS_TLV
            stream.readUint16Be(); // TLV value length, assumed 4
            hello->setHoldTime(stream.readUint16Be());
            hello->setTbit(stream.readBit());
            hello->setRbit(stream.readBit());
            stream.readNBitsToUint64Be(14); // reserved
            ldpPacket = hello;
            break;
        }
        case LABEL_REQUEST: {
            auto req = makeShared<LdpLabelRequest>();
            req->setFec(deserializeFecTlv(stream));
            ldpPacket = req;
            break;
        }
        case LABEL_MAPPING:
        case LABEL_WITHDRAW:
        case LABEL_RELEASE: {
            auto mapping = makeShared<LdpLabelMapping>();
            mapping->setFec(deserializeFecTlv(stream));
            stream.readBit(); // U-bit
            stream.readNBitsToUint64Be(15); // TLV type, assumed GENERIC_LABEL_TLV
            stream.readUint16Be(); // TLV value length, assumed 4
            mapping->setLabel(stream.readUint32Be());
            ldpPacket = mapping;
            break;
        }
        case NOTIFICATION: {
            auto notify = makeShared<LdpNotify>();
            stream.readBit(); // U-bit
            stream.readNBitsToUint64Be(15); // TLV type, assumed STATUS_TLV
            stream.readUint16Be(); // TLV value length, assumed 10
            stream.readBit(); // E-bit
            stream.readBit(); // F-bit
            notify->setStatus(stream.readNBitsToUint64Be(30));
            stream.readUint32Be(); // referenced Message ID: not tracked by this model
            stream.readUint16Be(); // referenced Message Type: ditto
            notify->setFec(deserializeFecTlv(stream));
            ldpPacket = notify;
            break;
        }
        case INITIALIZATION: {
            auto ini = makeShared<LdpIni>();
            stream.readBit(); // U-bit
            stream.readNBitsToUint64Be(15); // TLV type, assumed COMMON_SESSION_PARAMETERS_TLV
            stream.readUint16Be(); // TLV value length, assumed 16
            stream.readUint16Be(); // Protocol Version: redundant with the PDU header's version field, discarded
            ini->setKeepAliveTime(stream.readUint16Be());
            ini->setAbit(stream.readBit());
            ini->setDbit(stream.readBit());
            stream.readNBitsToUint64Be(14); // reserved
            ini->setPvLim(stream.readUint16Be());
            ini->setMaxPduLength(stream.readUint16Be());
            ini->setReceiverLsrId(stream.readIpv4Address());
            ini->setReceiverLabelSpace(stream.readUint16Be());
            ldpPacket = ini;
            break;
        }
        case KEEP_ALIVE: {
            // no message parameters (RFC 5036 Section 3.5.4)
            ldpPacket = makeShared<LdpKeepAlive>();
            break;
        }
        case ADDRESS:
        case ADDRESS_WITHDRAW: {
            auto addr = makeShared<LdpAddress>();
            addr->setIsWithdraw(type == ADDRESS_WITHDRAW);
            stream.readBit(); // U-bit
            stream.readNBitsToUint64Be(15); // TLV type, assumed ADDRESS_LIST_TLV
            uint16_t tlvValueLength = stream.readUint16Be();
            addr->setAddressFamily(stream.readUint16Be());
            int numAddresses = (tlvValueLength - 2) / 4;
            addr->setAddressesArraySize(numAddresses);
            for (int i = 0; i < numAddresses; ++i)
                addr->setAddresses(i, stream.readIpv4Address());
            ldpPacket = addr;
            break;
        }
        default: {
            auto unknown = makeShared<LdpPacket>();
            unknown->markIncorrect();
            ldpPacket = unknown;
            break;
        }
    }

    ldpPacket->setVersion(version);
    ldpPacket->setLsrId(lsrId);
    ldpPacket->setLabelSpace(labelSpace);
    ldpPacket->setType(type);
    ldpPacket->setMessageId(messageId);
    ldpPacket->setChunkLength(B(4) + B(pduLength));
    return ldpPacket;
}

} // namespace inet
