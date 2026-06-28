//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/lisp/LispSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {
namespace lisp {

Register_Serializer(LispHeader, LispHeaderSerializer);
Register_Serializer(LispControlMessage, LispControlMessageSerializer);
Register_Serializer(LispMapRequest, LispControlMessageSerializer);
Register_Serializer(LispMapReply, LispControlMessageSerializer);
Register_Serializer(LispMapRegister, LispControlMessageSerializer);
Register_Serializer(LispMapNotify, LispControlMessageSerializer);

// ---- AFI-encoded address: 2-byte address family followed by the address ----

B lispAfiAddressLength(const L3Address& address)
{
    return B(2) + B(address.getType() == L3Address::IPv6 ? 16 : 4);
}

static void writeAfiAddress(MemoryOutputStream& stream, const L3Address& address)
{
    if (address.getType() == L3Address::IPv6) {
        stream.writeUint16Be(2); // AFI = IPv6
        stream.writeIpv6Address(address.toIpv6());
    }
    else {
        stream.writeUint16Be(1); // AFI = IPv4 (also covers an unspecified address as 0.0.0.0)
        stream.writeIpv4Address(address.toIpv4());
    }
}

static L3Address readAfiAddress(MemoryInputStream& stream)
{
    uint16_t afi = stream.readUint16Be();
    if (afi == 2)
        return L3Address(stream.readIpv6Address());
    return L3Address(stream.readIpv4Address());
}

// ---- EID-to-RLOC record ----

B lispMapRecordLength(const LispMapRecord& record)
{
    B length = B(10) + lispAfiAddressLength(record.getEidPrefix());
    for (size_t i = 0; i < record.getLocatorsArraySize(); i++)
        length += B(6) + lispAfiAddressLength(record.getLocators(i).rloc);
    return length;
}

static void writeMapRecord(MemoryOutputStream& stream, const LispMapRecord& record)
{
    stream.writeUint32Be(record.getRecordTTL());
    stream.writeUint8(record.getLocatorsArraySize());
    stream.writeUint8(record.getEidMaskLength());
    stream.writeUint8(((record.getAct() & 0x07) << 5) | (record.getABit() ? 0x10 : 0));
    stream.writeUint8(0); // reserved
    stream.writeUint16Be(record.getMapVersion() & 0x0fff);
    writeAfiAddress(stream, record.getEidPrefix());
    for (size_t i = 0; i < record.getLocatorsArraySize(); i++) {
        const LispLocatorRecord& loc = record.getLocators(i);
        stream.writeUint8(loc.priority);
        stream.writeUint8(loc.weight);
        stream.writeUint8(loc.mpriority);
        stream.writeUint8(loc.mweight);
        stream.writeUint8(0); // unused flags octet
        stream.writeUint8((loc.localLocBit ? 0x04 : 0) | (loc.probedBit ? 0x02 : 0) | (loc.reachableBit ? 0x01 : 0));
        writeAfiAddress(stream, loc.rloc);
    }
}

static LispMapRecord readMapRecord(MemoryInputStream& stream)
{
    LispMapRecord record;
    record.setRecordTTL(stream.readUint32Be());
    uint8_t locatorCount = stream.readUint8();
    record.setEidMaskLength(stream.readUint8());
    uint8_t actAndFlags = stream.readUint8();
    record.setAct((actAndFlags >> 5) & 0x07);
    record.setABit((actAndFlags & 0x10) != 0);
    stream.readUint8(); // reserved
    record.setMapVersion(stream.readUint16Be() & 0x0fff);
    record.setEidPrefix(readAfiAddress(stream));
    record.setLocatorsArraySize(locatorCount);
    for (uint8_t i = 0; i < locatorCount; i++) {
        LispLocatorRecord loc;
        loc.priority = stream.readUint8();
        loc.weight = stream.readUint8();
        loc.mpriority = stream.readUint8();
        loc.mweight = stream.readUint8();
        stream.readUint8(); // unused flags octet
        uint8_t lpr = stream.readUint8();
        loc.localLocBit = (lpr & 0x04) != 0;
        loc.probedBit = (lpr & 0x02) != 0;
        loc.reachableBit = (lpr & 0x01) != 0;
        loc.rloc = readAfiAddress(stream);
        record.setLocators(i, loc);
    }
    return record;
}

// ---- LISP data-plane header (RFC 6830, section 5.3) ----

void LispHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const LispHeader>(chunk);
    // first octet: N|L|E|V|I flag bits followed by the 3-bit unused flags field
    stream.writeBit(header->getNonceBit());
    stream.writeBit(header->getLocStatBit());
    stream.writeBit(header->getEchoNonceBit());
    stream.writeBit(header->getVerMapBit());
    stream.writeBit(header->getInstanceBit());
    stream.writeNBitsOfUint64Be(header->getFlags(), 3);
    stream.writeUint24Be(header->getNonce());           // nonce / map-version (24 bits)
    stream.writeUint32Be(header->getInstanceId());      // instance id / locator-status-bits
}

const Ptr<Chunk> LispHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto header = makeShared<LispHeader>();
    header->setNonceBit(stream.readBit());
    header->setLocStatBit(stream.readBit());
    header->setEchoNonceBit(stream.readBit());
    header->setVerMapBit(stream.readBit());
    header->setInstanceBit(stream.readBit());
    header->setFlags(stream.readNBitsToUint64Be(3));
    header->setNonce(stream.readUint24Be());
    header->setInstanceId(stream.readUint32Be());
    return header;
}

// ---- Wire lengths of the control messages ----

B lispMapRequestLength(const LispMapRequest& msg)
{
    B length = B(12) + lispAfiAddressLength(msg.getSourceEid().address);
    for (size_t i = 0; i < msg.getItrRlocsArraySize(); i++)
        length += lispAfiAddressLength(msg.getItrRlocs(i).address);
    for (size_t i = 0; i < msg.getRecsArraySize(); i++)
        length += B(1) + lispAfiAddressLength(msg.getRecs(i).eidPrefix);
    if (msg.getMapDataBit())
        length += lispMapRecordLength(msg.getMapReply());
    return length;
}

B lispMapReplyLength(const LispMapReply& msg)
{
    B length = B(11);
    for (size_t i = 0; i < msg.getRecordsArraySize(); i++)
        length += lispMapRecordLength(msg.getRecords(i));
    return length;
}

B lispMapRegisterLength(const LispMapRegister& msg)
{
    B length = B(14) + B(msg.getAuthDataLen());
    for (size_t i = 0; i < msg.getRecordsArraySize(); i++)
        length += lispMapRecordLength(msg.getRecords(i));
    return length;
}

// ---- Control messages (RFC 6830, section 6), selected by the leading type octet ----

void LispControlMessageSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& base = staticPtrCast<const LispControlMessage>(chunk);
    uint8_t type = base->getType();
    switch (type) {
        case LISP_REQUEST: {
            const auto& msg = staticPtrCast<const LispMapRequest>(chunk);
            stream.writeUint8(type);
            stream.writeUint8((msg->getABit() ? 0x01 : 0) | (msg->getMapDataBit() ? 0x02 : 0) |
                    (msg->getProbeBit() ? 0x04 : 0) | (msg->getSmrBit() ? 0x08 : 0) |
                    (msg->getPitrBit() ? 0x10 : 0) | (msg->getSmrInvokedBit() ? 0x20 : 0));
            stream.writeUint8(msg->getRecsArraySize());
            stream.writeUint64Be(msg->getNonce());
            stream.writeUint8(msg->getItrRlocsArraySize());
            writeAfiAddress(stream, msg->getSourceEid().address);
            for (size_t i = 0; i < msg->getItrRlocsArraySize(); i++)
                writeAfiAddress(stream, msg->getItrRlocs(i).address);
            for (size_t i = 0; i < msg->getRecsArraySize(); i++) {
                stream.writeUint8(msg->getRecs(i).eidMaskLength);
                writeAfiAddress(stream, msg->getRecs(i).eidPrefix);
            }
            if (msg->getMapDataBit())
                writeMapRecord(stream, msg->getMapReply());
            break;
        }
        case LISP_REPLY: {
            const auto& msg = staticPtrCast<const LispMapReply>(chunk);
            stream.writeUint8(type);
            stream.writeUint8((msg->getProbeBit() ? 0x01 : 0) | (msg->getEchoNonceBit() ? 0x02 : 0) |
                    (msg->getSecBit() ? 0x04 : 0));
            stream.writeUint8(msg->getRecordsArraySize());
            stream.writeUint64Be(msg->getNonce());
            for (size_t i = 0; i < msg->getRecordsArraySize(); i++)
                writeMapRecord(stream, msg->getRecords(i));
            break;
        }
        case LISP_REGISTER:
        case LISP_NOTIFY: {
            const auto& msg = staticPtrCast<const LispMapRegister>(chunk);
            stream.writeUint8(type);
            stream.writeUint8((msg->getProxyBit() ? 0x01 : 0) | (msg->getMapNotifyBit() ? 0x02 : 0));
            stream.writeUint8(msg->getRecordsArraySize());
            stream.writeUint64Be(msg->getNonce());
            stream.writeUint8(msg->getKeyId());
            stream.writeUint16Be(msg->getAuthDataLen());
            const char *authData = msg->getAuthData();
            for (uint16_t i = 0; i < msg->getAuthDataLen(); i++)
                stream.writeByte((uint8_t)authData[i]);
            for (size_t i = 0; i < msg->getRecordsArraySize(); i++)
                writeMapRecord(stream, msg->getRecords(i));
            break;
        }
        default:
            throw cRuntimeError("LispControlMessageSerializer: cannot serialize unknown LISP message type %d", type);
    }
}

const Ptr<Chunk> LispControlMessageSerializer::deserialize(MemoryInputStream& stream) const
{
    uint8_t type = stream.readUint8();
    switch (type) {
        case LISP_REQUEST: {
            auto msg = makeShared<LispMapRequest>();
            uint8_t flags = stream.readUint8();
            msg->setABit((flags & 0x01) != 0);
            msg->setMapDataBit((flags & 0x02) != 0);
            msg->setProbeBit((flags & 0x04) != 0);
            msg->setSmrBit((flags & 0x08) != 0);
            msg->setPitrBit((flags & 0x10) != 0);
            msg->setSmrInvokedBit((flags & 0x20) != 0);
            msg->setRecordCount(stream.readUint8());
            msg->setNonce(stream.readUint64Be());
            uint8_t itrRlocCount = stream.readUint8();
            msg->setItrRlocCount(itrRlocCount);
            LispAfiAddr sourceEid;
            sourceEid.address = readAfiAddress(stream);
            msg->setSourceEid(sourceEid);
            msg->setItrRlocsArraySize(itrRlocCount);
            for (uint8_t i = 0; i < itrRlocCount; i++) {
                LispAfiAddr addr;
                addr.address = readAfiAddress(stream);
                msg->setItrRlocs(i, addr);
            }
            msg->setRecsArraySize(msg->getRecordCount());
            for (uint8_t i = 0; i < msg->getRecordCount(); i++) {
                LispEidRecord rec;
                rec.eidMaskLength = stream.readUint8();
                rec.eidPrefix = readAfiAddress(stream);
                msg->setRecs(i, rec);
            }
            if (msg->getMapDataBit())
                msg->setMapReply(readMapRecord(stream));
            return msg;
        }
        case LISP_REPLY: {
            auto msg = makeShared<LispMapReply>();
            uint8_t flags = stream.readUint8();
            msg->setProbeBit((flags & 0x01) != 0);
            msg->setEchoNonceBit((flags & 0x02) != 0);
            msg->setSecBit((flags & 0x04) != 0);
            msg->setRecordCount(stream.readUint8());
            msg->setNonce(stream.readUint64Be());
            msg->setRecordsArraySize(msg->getRecordCount());
            for (uint8_t i = 0; i < msg->getRecordCount(); i++)
                msg->setRecords(i, readMapRecord(stream));
            return msg;
        }
        case LISP_REGISTER:
        case LISP_NOTIFY: {
            Ptr<LispMapRegister> msg;
            if (type == LISP_NOTIFY)
                msg = makeShared<LispMapNotify>();
            else
                msg = makeShared<LispMapRegister>();
            uint8_t flags = stream.readUint8();
            msg->setProxyBit((flags & 0x01) != 0);
            msg->setMapNotifyBit((flags & 0x02) != 0);
            msg->setRecordCount(stream.readUint8());
            msg->setNonce(stream.readUint64Be());
            msg->setKeyId(stream.readUint8());
            uint16_t authDataLen = stream.readUint16Be();
            msg->setAuthDataLen(authDataLen);
            std::string authData;
            for (uint16_t i = 0; i < authDataLen; i++)
                authData += (char)stream.readByte();
            msg->setAuthData(authData.c_str());
            msg->setRecordsArraySize(msg->getRecordCount());
            for (uint8_t i = 0; i < msg->getRecordCount(); i++)
                msg->setRecords(i, readMapRecord(stream));
            return msg;
        }
        default:
            throw cRuntimeError("LispControlMessageSerializer: cannot deserialize unknown LISP message type %d", type);
    }
}

} // namespace lisp
} // namespace inet
