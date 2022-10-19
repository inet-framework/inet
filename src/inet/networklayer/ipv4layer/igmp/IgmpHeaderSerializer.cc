//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen, Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv4/IgmpHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/networklayer/ipv4/IgmpMessage_m.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h> // htonl, ntohl, ...
#endif // if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

namespace inet {

Register_Serializer(IgmpMessage, IgmpHeaderSerializer);
Register_Serializer(IgmpQuery, IgmpHeaderSerializer);
Register_Serializer(Igmpv1Query, IgmpHeaderSerializer);
Register_Serializer(Igmpv1Report, IgmpHeaderSerializer);
Register_Serializer(Igmpv2Query, IgmpHeaderSerializer);
Register_Serializer(Igmpv2Report, IgmpHeaderSerializer);
Register_Serializer(Igmpv2Leave, IgmpHeaderSerializer);
Register_Serializer(Igmpv3Query, IgmpHeaderSerializer);
Register_Serializer(Igmpv3Report, IgmpHeaderSerializer);

void IgmpHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& igmpMessage = staticPtrCast<const IgmpMessage>(chunk);
    IgmpType type = igmpMessage->getType();
    stream.writeByte(type);
    switch (type) {
        case IGMP_MEMBERSHIP_QUERY: {
            if (auto igmpv1Query = dynamicPtrCast<const Igmpv1Query>(igmpMessage))
                stream.writeByte(igmpv1Query->getUnused());
            else if (auto igmpv2Query = dynamicPtrCast<const Igmpv2Query>(igmpMessage))
                stream.writeByte(igmpv2Query->getMaxRespTimeCode());
            stream.writeUint16Be(igmpMessage->getCrc());
            stream.writeIpv4Address(check_and_cast<const IgmpQuery *>(igmpMessage.get())->getGroupAddress());
            if (auto igmpv3Query = dynamicPtrCast<const Igmpv3Query>(igmpMessage)) {
                ASSERT(igmpv3Query->getRobustnessVariable() <= 7);
                stream.writeNBitsOfUint64Be(igmpv3Query->getResv(), 4);
                stream.writeBit(igmpv3Query->getSuppressRouterProc());
                stream.writeNBitsOfUint64Be(igmpv3Query->getRobustnessVariable(), 3);
                stream.writeByte(igmpv3Query->getQueryIntervalCode());
                uint16_t numOfSources = igmpv3Query->getSourceList().size();
                stream.writeUint16Be(numOfSources);
                for (uint16_t i = 0; i < numOfSources; ++i)
                    stream.writeIpv4Address(igmpv3Query->getSourceList()[i]);
            }
            break;
        }
        case IGMPV1_MEMBERSHIP_REPORT: {
            auto igmpv1Report = dynamicPtrCast<const Igmpv1Report>(igmpMessage);
            stream.writeByte(igmpv1Report->getUnused());
            stream.writeUint16Be(igmpv1Report->getCrc());
            stream.writeIpv4Address(igmpv1Report->getGroupAddress());
            break;
        }
        case IGMPV2_MEMBERSHIP_REPORT: {
            auto igmpv2Report = dynamicPtrCast<const Igmpv2Report>(igmpMessage);
            stream.writeByte(igmpv2Report->getMaxRespTime());
            stream.writeUint16Be(igmpv2Report->getCrc());
            stream.writeIpv4Address(igmpv2Report->getGroupAddress());
            break;
        }
        case IGMPV2_LEAVE_GROUP: {
            auto igmpv2Leave = dynamicPtrCast<const Igmpv2Leave>(igmpMessage);
            stream.writeByte(igmpv2Leave->getMaxRespTime());
            stream.writeUint16Be(igmpv2Leave->getCrc());
            stream.writeIpv4Address(igmpv2Leave->getGroupAddress());
            break;
        }
        case IGMPV3_MEMBERSHIP_REPORT: {
            const Igmpv3Report *igmpv3Report = check_and_cast<const Igmpv3Report *>(igmpMessage.get());
            stream.writeByte(igmpv3Report->getResv1());
            stream.writeUint16Be(igmpv3Report->getCrc());
            stream.writeUint16Be(igmpv3Report->getResv2());
            unsigned int numOfRecords = igmpv3Report->getGroupRecordArraySize();
            stream.writeUint16Be(numOfRecords);
            for (unsigned int i = 0; i < numOfRecords; i++) {
                const GroupRecord& groupRecord = igmpv3Report->getGroupRecord(i);
                stream.writeByte(groupRecord.getRecordType());
                stream.writeByte(groupRecord.getAuxDataArraySize());
                stream.writeUint16Be(groupRecord.getSourceList().size());
                stream.writeIpv4Address(groupRecord.getGroupAddress());
                for (auto src : groupRecord.getSourceList()) {
                    stream.writeIpv4Address(src);
                }
                for (size_t i = 0; i < groupRecord.getAuxDataArraySize(); ++i) {
                    stream.writeUint32Be(groupRecord.getAuxData(i));
                }
            }
            break;
        }
        default:
            throw cRuntimeError("Can not serialize IGMP packet (%s): type %d not supported.", igmpMessage->getClassName(), type);
    }
}

const Ptr<Chunk> IgmpHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    B start = stream.getRemainingLength();
    unsigned char type = stream.readByte();
    unsigned char code = stream.readByte();
    uint16_t chksum = stream.readUint16Be();
    switch (type) {
        case IGMP_MEMBERSHIP_QUERY: {
            if (start == B(8)) {
                if (code == 0) {
                    auto igmpv1Query = makeShared<Igmpv1Query>();
                    igmpv1Query->setUnused(code);
                    igmpv1Query->setCrc(chksum);
                    igmpv1Query->setCrcMode(CRC_COMPUTED);
                    igmpv1Query->setGroupAddress(stream.readIpv4Address());
                    return igmpv1Query;
                }
                else {
                    auto igmpv2Query = makeShared<Igmpv2Query>();
                    igmpv2Query->setMaxRespTimeCode(code);
                    igmpv2Query->setCrc(chksum);
                    igmpv2Query->setCrcMode(CRC_COMPUTED);
                    igmpv2Query->setGroupAddress(stream.readIpv4Address());
                    return igmpv2Query;
                }
            }
            else {
                auto igmpv3Query = makeShared<Igmpv3Query>();
                igmpv3Query->setMaxRespTimeCode(code);
                igmpv3Query->setCrc(chksum);
                igmpv3Query->setCrcMode(CRC_COMPUTED);
                igmpv3Query->setGroupAddress(stream.readIpv4Address());
                igmpv3Query->setResv(stream.readNBitsToUint64Be(4));
                igmpv3Query->setSuppressRouterProc(stream.readBit());
                igmpv3Query->setRobustnessVariable(stream.readNBitsToUint64Be(3));
                igmpv3Query->setQueryIntervalCode(stream.readByte());
                uint16_t numOfSources = stream.readUint16Be();
                igmpv3Query->setChunkLength(B(12) + B(numOfSources * 4));
                for (uint16_t i = 0; i < numOfSources; ++i)
                    igmpv3Query->getSourceListForUpdate().push_back(stream.readIpv4Address());
                return igmpv3Query;
            }
        }
        case IGMPV1_MEMBERSHIP_REPORT:  {
            auto igmpv1Report = makeShared<Igmpv1Report>();
            igmpv1Report->setUnused(code);
            igmpv1Report->setCrc(chksum);
            igmpv1Report->setCrcMode(CRC_COMPUTED);
            igmpv1Report->setGroupAddress(stream.readIpv4Address());
            return igmpv1Report;
        }
        case IGMPV2_MEMBERSHIP_REPORT: {
            auto igmpv2Report = makeShared<Igmpv2Report>();
            igmpv2Report->setMaxRespTime(code);
            igmpv2Report->setCrc(chksum);
            igmpv2Report->setCrcMode(CRC_COMPUTED);
            igmpv2Report->setGroupAddress(stream.readIpv4Address());
            return igmpv2Report;
        }
        case IGMPV2_LEAVE_GROUP: {
            auto igmpv2Leave = makeShared<Igmpv2Leave>();
            igmpv2Leave->setMaxRespTime(code);
            igmpv2Leave->setCrc(chksum);
            igmpv2Leave->setCrcMode(CRC_COMPUTED);
            igmpv2Leave->setGroupAddress(stream.readIpv4Address());
            return igmpv2Leave;
        }
        case IGMPV3_MEMBERSHIP_REPORT: {
            auto igmpv3Report = makeShared<Igmpv3Report>();
            igmpv3Report->setResv1(code);
            igmpv3Report->setChunkLength(start);
            igmpv3Report->setCrc(chksum);
            igmpv3Report->setCrcMode(CRC_COMPUTED);
            igmpv3Report->setResv2(stream.readUint16Be());
            uint8_t numOfRecords = stream.readUint16Be();
            igmpv3Report->setGroupRecordArraySize(numOfRecords);
            for (uint16_t i = 0; i < numOfRecords; ++i) {
                GroupRecord groupRecord;
                groupRecord.setRecordType(stream.readByte());
                uint8_t auxDataLen = stream.readByte();
                groupRecord.setAuxDataArraySize(auxDataLen);
                uint8_t numOfSources = stream.readUint16Be();
                groupRecord.setGroupAddress(stream.readIpv4Address());
                for (uint8_t j = 0; j < numOfSources; j++) {
                    groupRecord.getSourceListForUpdate().push_back(stream.readIpv4Address());
                }
                for (size_t k = 0; k < auxDataLen; ++k) {
                    groupRecord.setAuxData(k, stream.readUint32Be());
                }
                igmpv3Report->setGroupRecord(i, groupRecord);
            }
            while (stream.getRemainingLength() > B(0))
                stream.readByte();
            return igmpv3Report;
        }
        default: {
            EV_ERROR << "IGMPSerializer: can not create IGMP packet: type " << type << " not supported\n";
            auto igmpMessage = makeShared<IgmpMessage>();
            igmpMessage->markIncorrect();
            return igmpMessage;
        }
    }
}

} // namespace inet

