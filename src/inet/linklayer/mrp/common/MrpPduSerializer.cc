//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/mrp/common/MrpPduSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/mrp/common/MrpPdu_m.h"

namespace inet {

Register_Serializer(tlvHeader, mrpTlvSerializer);
Register_Serializer(testFrame, mrpTlvSerializer);
Register_Serializer(topologyChangeFrame, mrpTlvSerializer);
Register_Serializer(linkChangeFrame, mrpTlvSerializer);
Register_Serializer(inLinkChangeFrame, mrpTlvSerializer);
Register_Serializer(inLinkStatusPollFrame, mrpTlvSerializer);
Register_Serializer(inTopologyChangeFrame, mrpTlvSerializer);
Register_Serializer(commonHeader, mrpTlvSerializer);
Register_Serializer(optionHeader, mrpTlvSerializer);
Register_Serializer(subTlvHeader, mrpSubTlvSerializer);
Register_Serializer(manufacturerFktHeader, mrpSubTlvSerializer);
Register_Serializer(subTlvTestFrame, mrpSubTlvSerializer);
Register_Serializer(mrpVersionField, mrpVersionFieldSerializer);


void mrpVersionFieldSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& version = staticPtrCast<const mrpVersionField>(chunk);
    stream.writeUint16Be(version->getVersion());
}

void mrpTlvSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& tlv = staticPtrCast<const tlvHeader>(chunk);
    stream.writeUint8(tlv->getHeaderType());
    stream.writeUint8(tlv->getHeaderLength());

    switch (tlv->getHeaderType()){
    case END:
        break;

    case COMMON:
    {
        const auto& common = staticPtrCast<const commonHeader>(chunk);
        stream.writeUint16Be(common->getSequenceID());
        stream.writeUint64Be(common->getUuid0());
        stream.writeUint64Be(common->getUuid1());
        break;
    }
    case TEST:
    {
        const auto& tf = staticPtrCast<const testFrame>(chunk);
        stream.writeUint16Be(tf->getPrio());
        stream.writeMacAddress(tf->getSa());
        stream.writeUint16Be(tf->getPortRole());
        stream.writeUint16Be(tf->getRingState());
        stream.writeUint16Be(tf->getTransition());
        stream.writeUint32Be(tf->getTimeStamp());
        break;
    }
    case TOPOLOGYCHANGE:
    {
        const auto& tcf = staticPtrCast<const topologyChangeFrame>(chunk);
        stream.writeUint16Be(tcf->getPrio());
        stream.writeMacAddress(tcf->getSa());
        stream.writeUint16Be(tcf->getPortRole());
        stream.writeUint16Be(tcf->getInterval());
        break;
    }
    case LINKDOWN:
    case LINKUP:
    {
        const auto& lcf = staticPtrCast<const linkChangeFrame>(chunk);
        stream.writeMacAddress(lcf->getSa());
        stream.writeUint16Be(lcf->getPortRole());
        stream.writeUint16Be(lcf->getInterval());
        stream.writeUint16Be(lcf->getBlocked());
        break;
    }
    case INTEST:
    {
        const auto& itf = staticPtrCast<const inTestFrame>(chunk);
        stream.writeUint16Be(itf->getInID());
        stream.writeMacAddress(itf->getSa());
        stream.writeUint16Be(itf->getPortRole());
        stream.writeUint16Be(itf->getInState());
        stream.writeUint16Be(itf->getTransition());
        stream.writeUint32Be(itf->getTimeStamp());
        break;
    }
    case INTOPOLOGYCHANGE:
    {
        const auto& itcf = staticPtrCast<const inTopologyChangeFrame>(chunk);
        stream.writeMacAddress(itcf->getSa());
        stream.writeUint16Be(itcf->getInID());
        stream.writeUint16Be(itcf->getInterval());
        break;
    }
    case INLINKDOWN:
    case INLINKUP:
    {
        const auto& ilc = staticPtrCast<const inLinkChangeFrame>(chunk);
        stream.writeMacAddress(ilc->getSa());
        stream.writeUint16Be(ilc->getPortRole());
        stream.writeUint16Be(ilc->getInID());
        stream.writeUint16Be(ilc->getInterval());
        stream.writeUint16Be(ilc->getLinkInfo());
        break;
    }
    case INLINKSTATUSPOLL:
    {
        const auto& ilsp = staticPtrCast<const inLinkStatusPollFrame>(chunk);
        stream.writeMacAddress(ilsp->getSa());
        stream.writeUint16Be(ilsp->getPortRole());
        stream.writeUint16Be(ilsp->getInID());
        break;
    }
    case OPTION:
    {
        const auto& oh = staticPtrCast<const optionHeader>(chunk);
        stream.writeUint32Be(oh->getOuiType());
        stream.writeUint8(oh->getEd1Type());
        break;
    }
    default:
        throw cRuntimeError("Unknown Header TYPE value: %d", static_cast<int>(tlv->getHeaderType()));

    }
}

void mrpSubTlvSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& subTlv = staticPtrCast<const subTlvHeader>(chunk);
    stream.writeUint8(subTlv->getSubType());
    stream.writeUint8(subTlv->getSubHeaderLength());

    switch (subTlv->getSubType()){
    case RESERVED: //fallthrough to Automgr
    case AUTOMGR:
        break;
    case TEST_MGR_NACK: //fallthrough to Test_PROPAGATE
    case TEST_PROPAGATE:
    {
        const auto& stf = staticPtrCast<const subTlvTestFrame>(chunk);
        stream.writeUint16Be(stf->getPrio());
        stream.writeMacAddress(stf->getSa());
        stream.writeUint16Be(stf->getOtherMRMPrio());
        stream.writeMacAddress(stf->getOtherMRMSa());
        break;
    }
    default:
        throw cRuntimeError("Unknown Header TYPE value: %d", static_cast<int>(subTlv->getSubType()));
    }
}

const Ptr<Chunk> mrpVersionFieldSerializer::deserialize(MemoryInputStream& stream) const
{
    auto mrpVersion = makeShared<mrpVersionField>();
    mrpVersion->setVersion(stream.readUint16Be());
    return mrpVersion;
}

const Ptr<Chunk> mrpTlvSerializer::deserialize(MemoryInputStream& stream) const
{
    auto headerType = static_cast<tlvHeaderType>(stream.readUint16Be());
    switch (headerType) {
    case END:
    {
        auto tlv = makeShared<tlvHeader>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        return tlv;
    }
    case COMMON:
    {
        auto tlv = makeShared<commonHeader>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        tlv->setSequenceID(stream.readUint16Be());
        tlv->setUuid0(stream.readUint64Be());
        tlv->setUuid1(stream.readUint64Be());
        return tlv;
    }
    case TEST:
    {
        auto tlv = makeShared<testFrame>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        tlv->setPrio(stream.readUint16Be());
        tlv->setSa(stream.readMacAddress());
        tlv->setPortRole(stream.readUint16Be());
        tlv->setRingState(stream.readUint16Be());
        tlv->setTransition(stream.readUint16Be());
        tlv->setTimeStamp(stream.readUint32Be());
        return tlv;
    }

    case TOPOLOGYCHANGE:
    {
        auto tlv = makeShared<topologyChangeFrame>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        tlv->setPrio(stream.readUint16Be());
        tlv->setSa(stream.readMacAddress());
        tlv->setPortRole(stream.readUint16Be());
        tlv->setInterval(stream.readUint16Be());
        return tlv;
    }

    case LINKDOWN:
    case LINKUP:
    {
        auto tlv = makeShared<linkChangeFrame>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        tlv->setSa(stream.readMacAddress());
        tlv->setPortRole(stream.readUint16Be());
        tlv->setInterval(stream.readUint16Be());
        tlv->setBlocked(stream.readUint16Be());
        return tlv;
    }

    case INTEST:
    {
        auto tlv = makeShared<inTestFrame>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        tlv->setInID(stream.readUint16Be());
        tlv->setSa(stream.readMacAddress());
        tlv->setPortRole(stream.readUint16Be());
        tlv->setInState(stream.readUint16Be());
        tlv->setTransition(stream.readUint16Be());
        tlv->setTimeStamp(stream.readUint32Be());
        return tlv;
    }

    case INTOPOLOGYCHANGE:
    {
        auto tlv = makeShared<inTopologyChangeFrame>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        tlv->setSa(stream.readMacAddress());
        tlv->setInID(stream.readUint16Be());
        tlv->setInterval(stream.readUint16Be());
        return tlv;
    }

    case INLINKDOWN:
    case INLINKUP:
    {
        auto tlv = makeShared<inLinkChangeFrame>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        tlv->setSa(stream.readMacAddress());
        tlv->setPortRole(stream.readUint16Be());
        tlv->setInID(stream.readUint16Be());
        tlv->setInterval(stream.readUint16Be());
        tlv->setLinkInfo(stream.readUint16Be());
        return tlv;
    }

    case INLINKSTATUSPOLL:
    {
        auto tlv = makeShared<inLinkStatusPollFrame>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        tlv->setSa(stream.readMacAddress());
        tlv->setPortRole(stream.readUint16Be());
        tlv->setInID(stream.readUint16Be());
        return tlv;
    }

    case OPTION:
    {
        auto tlv = makeShared<optionHeader>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        tlv->setOuiType(static_cast<mrpOuiType> (stream.readUint24Be()));
        tlv->setEd1Type(stream.readUint8());
        return tlv;
    }

    default:
        throw cRuntimeError("Unknown Header TYPE value: %d", static_cast<int>(headerType));
    }
}

const Ptr<Chunk> mrpSubTlvSerializer::deserialize(MemoryInputStream& stream) const
{
    auto subType = static_cast<subTlvHeaderType> (stream.readUint8());
    switch (subType){
    case RESERVED:
    {
        auto subTlv = makeShared<manufacturerFktHeader>();
        subTlv->setSubType(subType);
        subTlv->setSubHeaderLength(stream.readUint8());
        return subTlv;
    }
    case AUTOMGR:
    {
        auto subTlv = makeShared<subTlvHeader>();
        subTlv->setSubType(subType);
        subTlv->setSubHeaderLength(stream.readUint8());
        return subTlv;
    }
    case TEST_MGR_NACK:
    case TEST_PROPAGATE:
    {
        auto subTlv = makeShared<subTlvTestFrame>();
        subTlv->setSubType(subType);
        subTlv->setSubHeaderLength(stream.readUint8());
        subTlv->setPrio(stream.readUint16Be());
        subTlv->setSa(stream.readMacAddress());
        subTlv->setOtherMRMPrio(stream.readUint16Be());
        subTlv->setOtherMRMSa(stream.readMacAddress());
        return subTlv;
    }
    default:
        throw cRuntimeError("Unknown Header TYPE value: %d", static_cast<int>(subType));
    }
}

} // namespace inet

