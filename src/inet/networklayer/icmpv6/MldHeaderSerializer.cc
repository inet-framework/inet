//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/icmpv6/MldHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/networklayer/icmpv6/MldMessage_m.h"
#include "inet/networklayer/icmpv6/Mldv2Message_m.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h> // htonl, ntohl, ...
#endif // if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

namespace inet {

Register_Serializer(MldMessage, MldHeaderSerializer);
Register_Serializer(MldQuery, MldHeaderSerializer);
Register_Serializer(MldReport, MldHeaderSerializer);
Register_Serializer(MldDone, MldHeaderSerializer);
Register_Serializer(Mldv2Query, MldHeaderSerializer);
Register_Serializer(Mldv2Report, MldHeaderSerializer);

void MldHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    // MLDv2 Report (RFC 3810, ICMPv6 type 143) has its own layout (a list of records)
    if (auto report = dynamicPtrCast<const Mldv2Report>(chunk)) {
        stream.writeByte(report->getType());                  // type (1 byte)
        stream.writeByte(report->getCode());                  // reserved (1 byte)
        stream.writeUint16Be(report->getChksum());            // checksum (2 bytes)
        stream.writeUint16Be(report->getReserved());          // reserved (2 bytes)
        unsigned int numRecords = report->getMulticastAddressRecordArraySize();
        stream.writeUint16Be(numRecords);                     // Nr of Multicast Address Records (2 bytes)
        for (unsigned int i = 0; i < numRecords; ++i) {
            const Mldv2MulticastAddressRecord& rec = report->getMulticastAddressRecord(i);
            stream.writeByte(rec.getRecordType());            // Record Type (1 byte)
            stream.writeByte(rec.getAuxDataArraySize());      // Aux Data Len (1 byte, in 32-bit words)
            stream.writeUint16Be(rec.getSourceList().size()); // Nr of Sources (2 bytes)
            stream.writeIpv6Address(rec.getGroupAddress());   // Multicast Address (16 bytes)
            for (auto& src : rec.getSourceList())
                stream.writeIpv6Address(src);                 // Source Address (16 bytes each)
            for (size_t k = 0; k < rec.getAuxDataArraySize(); ++k)
                stream.writeUint32Be(rec.getAuxData(k));      // Auxiliary Data
        }
        return;
    }

    // MLDv1 messages and the MLDv2 Query share the fixed 24-byte MldMessage prefix
    const auto& msg = staticPtrCast<const MldMessage>(chunk);
    stream.writeByte(msg->getType());                      // type (1 byte)
    stream.writeByte(msg->getCode());                      // code (1 byte, always 0)
    stream.writeUint16Be(msg->getChksum());               // checksum (2 bytes)
    stream.writeUint16Be(msg->getMaxRespDelay());         // Maximum Response Code/Delay (2 bytes)
    stream.writeUint16Be(msg->getReserved());             // reserved (2 bytes)
    stream.writeIpv6Address(msg->getMulticastAddress());  // Multicast Address (16 bytes)

    if (auto q2 = dynamicPtrCast<const Mldv2Query>(chunk)) {
        ASSERT(q2->getRobustnessVariable() <= 7);
        stream.writeUint4(q2->getResv());                              // Resv (4 bits)
        stream.writeBit(q2->getSuppressRouterProc());                  // S flag (1 bit)
        stream.writeNBitsOfUint64Be(q2->getRobustnessVariable(), 3);   // QRV (3 bits)
        stream.writeByte(q2->getQueryIntervalCode());                  // QQIC (1 byte)
        uint16_t numSources = q2->getSourceList().size();
        stream.writeUint16Be(numSources);                              // Nr of Sources (2 bytes)
        for (uint16_t i = 0; i < numSources; ++i)
            stream.writeIpv6Address(q2->getSourceList()[i]);           // Source Address (16 bytes each)
    }
}

const Ptr<Chunk> MldHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    B start = stream.getRemainingLength();
    uint8_t type = stream.readByte();
    uint8_t code = stream.readByte();
    uint16_t chksum = stream.readUint16Be();

    // MLDv2 Report (type 143)
    if (type == ICMPv6_MLDv2_REPORT) {
        auto report = makeShared<Mldv2Report>();
        report->setType(ICMPv6_MLDv2_REPORT);
        report->setCode(code);
        report->setChksum(chksum);
        report->setChecksumMode(CHECKSUM_COMPUTED);
        report->setReserved(stream.readUint16Be());
        uint16_t numRecords = stream.readUint16Be();
        report->setMulticastAddressRecordArraySize(numRecords);
        for (uint16_t i = 0; i < numRecords; ++i) {
            Mldv2MulticastAddressRecord rec;
            rec.setRecordType(stream.readByte());
            uint8_t auxDataLen = stream.readByte();
            rec.setAuxDataArraySize(auxDataLen);
            uint16_t numSources = stream.readUint16Be();
            rec.setGroupAddress(stream.readIpv6Address());
            for (uint16_t j = 0; j < numSources; ++j)
                rec.getSourceListForUpdate().push_back(stream.readIpv6Address());
            for (size_t k = 0; k < auxDataLen; ++k)
                rec.setAuxData(k, stream.readUint32Be());
            report->setMulticastAddressRecord(i, rec);
        }
        report->setChunkLength(start);
        return report;
    }

    // common 24-byte MLDv1 / MLDv2-Query prefix
    uint16_t maxRespDelay = stream.readUint16Be();
    uint16_t reserved = stream.readUint16Be();
    Ipv6Address mcastAddr = stream.readIpv6Address();

    // MLDv2 Query (type 130, longer than the fixed 24-byte MLDv1 Query)
    if (type == ICMPv6_MLD_QUERY && start > B(24)) {
        auto q = makeShared<Mldv2Query>();
        q->setType(ICMPv6_MLD_QUERY);
        q->setCode(code);
        q->setChksum(chksum);
        q->setChecksumMode(CHECKSUM_COMPUTED);
        q->setMaxRespDelay(maxRespDelay);
        q->setReserved(reserved);
        q->setMulticastAddress(mcastAddr);
        q->setResv(stream.readUint4());
        q->setSuppressRouterProc(stream.readBit());
        q->setRobustnessVariable(stream.readNBitsToUint64Be(3));
        q->setQueryIntervalCode(stream.readByte());
        uint16_t numSources = stream.readUint16Be();
        for (uint16_t i = 0; i < numSources; ++i)
            q->getSourceListForUpdate().push_back(stream.readIpv6Address());
        q->setChunkLength(B(28) + B(numSources * 16));
        return q;
    }

    // MLDv1 Query/Report/Done (fixed 24 bytes)
    Ptr<MldMessage> msg;
    switch (type) {
        case ICMPv6_MLD_QUERY:
            msg = makeShared<MldQuery>();
            break;
        case ICMPv6_MLD_REPORT:
            msg = makeShared<MldReport>();
            break;
        case ICMPv6_MLD_DONE:
            msg = makeShared<MldDone>();
            break;
        default: {
            EV_ERROR << "MldHeaderSerializer: cannot parse MLD packet: type " << (int)type << " not supported\n";
            auto unknown = makeShared<MldMessage>();
            unknown->setChunkLength(B(24)); // 24 bytes already consumed; keep length in sync so later chunks parse correctly
            unknown->markIncorrect();
            return unknown;
        }
    }
    msg->setType(static_cast<Icmpv6Type>(type));
    msg->setCode(code);
    msg->setChksum(chksum);
    msg->setChecksumMode(CHECKSUM_COMPUTED);
    msg->setMaxRespDelay(maxRespDelay);
    msg->setReserved(reserved);
    msg->setMulticastAddress(mcastAddr);
    return msg;
}

} // namespace inet
