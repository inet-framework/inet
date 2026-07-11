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
    HOP_COUNT_TLV = 0x0103,
    PATH_VECTOR_TLV = 0x0104,
    GENERIC_LABEL_TLV = 0x0200,
    STATUS_TLV = 0x0300,
    COMMON_HELLO_PARAMETERS_TLV = 0x0400,
    COMMON_SESSION_PARAMETERS_TLV = 0x0500,
};

void LdpPacketSerializer::serializeFecTlv(MemoryOutputStream& stream, const FecTlv& fec)
{
    // Dual-stack (Workstream F3 Phase 5, RFC 7552 Section 3.1): the FEC Element's
    // Address Family selects IPv4 (1, a 4-byte prefix) or IPv6 (2, a 16-byte prefix);
    // this is the ONLY on-wire change RFC 7552 needs for this TLV.
    bool isV6 = fec.addr.getType() == L3Address::IPv6;
    stream.writeBit(false); // U-bit
    stream.writeNBitsOfUint64Be(FEC_TLV, 15);
    stream.writeUint16Be(isV6 ? 20 : 8); // TLV value length: 1(elem type) + 2(addr family) + 1(prelen) + 4 or 16(prefix)
    stream.writeByte(2); // FEC Element Type = 2 (Address Prefix)
    if (isV6) {
        stream.writeUint16Be(2); // Address Family = 2 (IPv6, RFC 7552 Section 3.1)
        stream.writeByte(fec.length);
        stream.writeIpv6Address(fec.addr.toIpv6());
    }
    else {
        stream.writeUint16Be(1); // Address Family = 1 (IP)
        stream.writeByte(fec.length);
        stream.writeIpv4Address(fec.addr.toIpv4());
    }
}

FecTlv LdpPacketSerializer::deserializeFecTlv(MemoryInputStream& stream)
{
    stream.readBit(); // U-bit
    stream.readNBitsToUint64Be(15); // TLV type, assumed FEC_TLV (dispatch already selected this branch)
    stream.readUint16Be(); // TLV value length: derived from the Address Family below, not used directly
    stream.readByte(); // FEC Element Type, assumed 2 (Address Prefix)
    uint16_t addressFamily = stream.readUint16Be(); // RFC 5036/7552: 1 = IPv4, 2 = IPv6
    FecTlv fec;
    fec.length = stream.readByte();
    if (addressFamily == 2)
        fec.addr = stream.readIpv6Address();
    else
        fec.addr = stream.readIpv4Address();
    return fec;
}

void LdpPacketSerializer::serializeLoopDetectionTlvs(MemoryOutputStream& stream, uint8_t hopCount, const std::vector<Ipv4Address>& pathVector)
{
    // Hop Count TLV (RFC 5036 Section 3.4.4)
    stream.writeBit(false);
    stream.writeNBitsOfUint64Be(HOP_COUNT_TLV, 15);
    stream.writeUint16Be(1); // value length: HC Value (1 octet)
    stream.writeByte(hopCount);

    // Path Vector TLV (RFC 5036 Section 3.4.5)
    stream.writeBit(false);
    stream.writeNBitsOfUint64Be(PATH_VECTOR_TLV, 15);
    stream.writeUint16Be(4 * pathVector.size());
    for (auto& lsrId : pathVector)
        stream.writeIpv4Address(lsrId);
}

void LdpPacketSerializer::deserializeLoopDetectionTlvs(MemoryInputStream& stream, uint8_t& hopCount, std::vector<Ipv4Address>& pathVector)
{
    stream.readBit(); // U-bit
    stream.readNBitsToUint64Be(15); // TLV type, assumed HOP_COUNT_TLV
    stream.readUint16Be(); // TLV value length, assumed 1
    hopCount = stream.readByte();

    stream.readBit(); // U-bit
    stream.readNBitsToUint64Be(15); // TLV type, assumed PATH_VECTOR_TLV
    uint16_t pvValueLength = stream.readUint16Be();
    int n = pvValueLength / 4;
    for (int i = 0; i < n; ++i)
        pathVector.push_back(stream.readIpv4Address());
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
            if (req->getHasLoopDetection()) {
                std::vector<Ipv4Address> pathVector;
                for (size_t i = 0; i < req->getPathVectorArraySize(); ++i)
                    pathVector.push_back(req->getPathVector(i));
                serializeLoopDetectionTlvs(stream, req->getHopCount(), pathVector);
            }
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
            // RFC 5036 Section 2.8: loop-detection TLVs (LABEL_MAPPING only -- see
            // Ldp::sendMapping, which never sets hasLoopDetection for a WITHDRAW/RELEASE)
            if (mapping->getHasLoopDetection()) {
                std::vector<Ipv4Address> pathVector;
                for (size_t i = 0; i < mapping->getPathVectorArraySize(); ++i)
                    pathVector.push_back(mapping->getPathVector(i));
                serializeLoopDetectionTlvs(stream, mapping->getHopCount(), pathVector);
            }
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
    // Message Length (RFC 5036 Section 3.6): cumulative length of the Message ID
    // plus the Mandatory/Optional Parameters that follow. Redundant with pduLength
    // given one-message-per-PDU for determining the OVERALL packet boundary, but
    // put to real use below for LABEL_REQUEST/LABEL_MAPPING: it is what tells us
    // whether the optional loop-detection TLVs (RFC 5036 Section 2.8) are present,
    // since inferring that from how much data remains in the underlying stream
    // would be wrong -- this message need not be the last one in the byte stream.
    uint16_t messageLength = stream.readUint16Be();
    uint32_t messageId = stream.readUint32Be();
    B paramsLength = B(messageLength) - B(4); // exclude the Message ID's own 4 bytes
    B messageStart = B(stream.getPosition());

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
            // RFC 5036 Section 2.8: optional Hop Count + Path Vector TLVs, present iff
            // the Message Length accounts for more than the mandatory FEC TLV just read
            if (paramsLength - (B(stream.getPosition()) - messageStart) > B(0)) {
                uint8_t hopCount;
                std::vector<Ipv4Address> pathVector;
                deserializeLoopDetectionTlvs(stream, hopCount, pathVector);
                req->setHasLoopDetection(true);
                req->setHopCount(hopCount);
                req->setPathVectorArraySize(pathVector.size());
                for (size_t i = 0; i < pathVector.size(); ++i)
                    req->setPathVector(i, pathVector[i]);
            }
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
            // RFC 5036 Section 2.8: optional Hop Count + Path Vector TLVs (LABEL_MAPPING
            // only -- see Ldp::sendMapping); present iff the Message Length accounts for
            // more than the mandatory FEC + Generic Label TLVs just read
            if (paramsLength - (B(stream.getPosition()) - messageStart) > B(0)) {
                uint8_t hopCount;
                std::vector<Ipv4Address> pathVector;
                deserializeLoopDetectionTlvs(stream, hopCount, pathVector);
                mapping->setHasLoopDetection(true);
                mapping->setHopCount(hopCount);
                mapping->setPathVectorArraySize(pathVector.size());
                for (size_t i = 0; i < pathVector.size(); ++i)
                    mapping->setPathVector(i, pathVector[i]);
            }
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
