// Copyright (C) 2024 Daniel Zeitler
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "inet/linklayer/mrp/common/MrpPduSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/mrp/common/MrpPdu_m.h"

namespace inet {

Register_Serializer(TlvHeader, MrpTlvSerializer);
Register_Serializer(TestFrame, MrpTlvSerializer);
Register_Serializer(TopologyChangeFrame, MrpTlvSerializer);
Register_Serializer(LinkChangeFrame, MrpTlvSerializer);
Register_Serializer(InLinkChangeFrame, MrpTlvSerializer);
Register_Serializer(InLinkStatusPollFrame, MrpTlvSerializer);
Register_Serializer(InTopologyChangeFrame, MrpTlvSerializer);
Register_Serializer(CommonHeader, MrpTlvSerializer);
Register_Serializer(OptionHeader, MrpTlvSerializer);
Register_Serializer(SubTlvHeader, MrpSubTlvSerializer);
Register_Serializer(ManufacturerFktHeader, MrpSubTlvSerializer);
Register_Serializer(SubTlvTestFrame, MrpSubTlvSerializer);
Register_Serializer(MrpVersionField, MrpVersionFieldSerializer);

void MrpVersionFieldSerializer::serialize(MemoryOutputStream &stream, const Ptr<const Chunk> &chunk) const {
    const auto &version = staticPtrCast<const MrpVersionField>(chunk);
    stream.writeUint16Be(version->getVersion());
}

void MrpTlvSerializer::serialize(MemoryOutputStream &stream, const Ptr<const Chunk> &chunk) const {
    const auto &tlv = staticPtrCast<const TlvHeader>(chunk);
    stream.writeUint8(tlv->getHeaderType());
    stream.writeUint8(tlv->getHeaderLength());

    switch (tlv->getHeaderType()) {
    case END:
        break;

    case COMMON: {
        const auto &common = staticPtrCast<const CommonHeader>(chunk);
        stream.writeUint16Be(common->getSequenceID());
        stream.writeUint64Be(common->getUuid0());
        stream.writeUint64Be(common->getUuid1());
        break;
    }
    case TEST: {
        const auto &tf = staticPtrCast<const TestFrame>(chunk);
        stream.writeUint16Be(tf->getPrio());
        stream.writeMacAddress(tf->getSa());
        stream.writeUint16Be(tf->getPortRole());
        stream.writeUint16Be(tf->getRingState());
        stream.writeUint16Be(tf->getTransition());
        stream.writeUint32Be(tf->getTimeStamp());
        break;
    }
    case TOPOLOGYCHANGE: {
        const auto &tcf = staticPtrCast<const TopologyChangeFrame>(chunk);
        stream.writeUint16Be(tcf->getPrio());
        stream.writeMacAddress(tcf->getSa());
        stream.writeUint16Be(tcf->getPortRole());
        stream.writeUint16Be(tcf->getInterval());
        break;
    }
    case LINKDOWN:
    case LINKUP: {
        const auto &lcf = staticPtrCast<const LinkChangeFrame>(chunk);
        stream.writeMacAddress(lcf->getSa());
        stream.writeUint16Be(lcf->getPortRole());
        stream.writeUint16Be(lcf->getInterval());
        stream.writeUint16Be(lcf->getBlocked());
        break;
    }
    case INTEST: {
        const auto &itf = staticPtrCast<const InTestFrame>(chunk);
        stream.writeUint16Be(itf->getInID());
        stream.writeMacAddress(itf->getSa());
        stream.writeUint16Be(itf->getPortRole());
        stream.writeUint16Be(itf->getInState());
        stream.writeUint16Be(itf->getTransition());
        stream.writeUint32Be(itf->getTimeStamp());
        break;
    }
    case INTOPOLOGYCHANGE: {
        const auto &itcf = staticPtrCast<const InTopologyChangeFrame>(chunk);
        stream.writeMacAddress(itcf->getSa());
        stream.writeUint16Be(itcf->getInID());
        stream.writeUint16Be(itcf->getInterval());
        break;
    }
    case INLINKDOWN:
    case INLINKUP: {
        const auto &ilc = staticPtrCast<const InLinkChangeFrame>(chunk);
        stream.writeMacAddress(ilc->getSa());
        stream.writeUint16Be(ilc->getPortRole());
        stream.writeUint16Be(ilc->getInID());
        stream.writeUint16Be(ilc->getInterval());
        stream.writeUint16Be(ilc->getLinkInfo());
        break;
    }
    case INLINKSTATUSPOLL: {
        const auto &ilsp = staticPtrCast<const InLinkStatusPollFrame>(chunk);
        stream.writeMacAddress(ilsp->getSa());
        stream.writeUint16Be(ilsp->getPortRole());
        stream.writeUint16Be(ilsp->getInID());
        break;
    }
    case OPTION: {
        const auto &oh = staticPtrCast<const OptionHeader>(chunk);
        stream.writeUint32Be(oh->getOuiType());
        stream.writeUint8(oh->getEd1Type());
        break;
    }
    default:
        throw cRuntimeError("Unknown Header TYPE value: %d",
                static_cast<int>(tlv->getHeaderType()));

    }
}

void MrpSubTlvSerializer::serialize(MemoryOutputStream &stream, const Ptr<const Chunk> &chunk) const {
    const auto &subTlv = staticPtrCast<const SubTlvHeader>(chunk);
    stream.writeUint8(subTlv->getSubType());
    stream.writeUint8(subTlv->getSubHeaderLength());

    switch (subTlv->getSubType()) {
    case RESERVED: //fallthrough to Automgr
    case AUTOMGR:
        break;
    case TEST_MGR_NACK: //fallthrough to Test_PROPAGATE
    case TEST_PROPAGATE: {
        const auto &stf = staticPtrCast<const SubTlvTestFrame>(chunk);
        stream.writeUint16Be(stf->getPrio());
        stream.writeMacAddress(stf->getSa());
        stream.writeUint16Be(stf->getOtherMRMPrio());
        stream.writeMacAddress(stf->getOtherMRMSa());
        break;
    }
    default:
        throw cRuntimeError("Unknown Header TYPE value: %d",
                static_cast<int>(subTlv->getSubType()));
    }
}

const Ptr<Chunk> MrpVersionFieldSerializer::deserialize(MemoryInputStream &stream) const {
    auto mrpVersion = makeShared<MrpVersionField>();
    mrpVersion->setVersion(stream.readUint16Be());
    return mrpVersion;
}

const Ptr<Chunk> MrpTlvSerializer::deserialize(MemoryInputStream &stream) const {
    auto headerType = static_cast<TlvHeaderType>(stream.readUint16Be());
    switch (headerType) {
    case END: {
        auto tlv = makeShared<TlvHeader>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        return tlv;
    }
    case COMMON: {
        auto tlv = makeShared<CommonHeader>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        tlv->setSequenceID(stream.readUint16Be());
        tlv->setUuid0(stream.readUint64Be());
        tlv->setUuid1(stream.readUint64Be());
        return tlv;
    }
    case TEST: {
        auto tlv = makeShared<TestFrame>();
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
    case TOPOLOGYCHANGE: {
        auto tlv = makeShared<TopologyChangeFrame>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        tlv->setPrio(stream.readUint16Be());
        tlv->setSa(stream.readMacAddress());
        tlv->setPortRole(stream.readUint16Be());
        tlv->setInterval(stream.readUint16Be());
        return tlv;
    }
    case LINKDOWN:
    case LINKUP: {
        auto tlv = makeShared<LinkChangeFrame>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        tlv->setSa(stream.readMacAddress());
        tlv->setPortRole(stream.readUint16Be());
        tlv->setInterval(stream.readUint16Be());
        tlv->setBlocked(stream.readUint16Be());
        return tlv;
    }
    case INTEST: {
        auto tlv = makeShared<InTestFrame>();
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
    case INTOPOLOGYCHANGE: {
        auto tlv = makeShared<InTopologyChangeFrame>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        tlv->setSa(stream.readMacAddress());
        tlv->setInID(stream.readUint16Be());
        tlv->setInterval(stream.readUint16Be());
        return tlv;
    }
    case INLINKDOWN:
    case INLINKUP: {
        auto tlv = makeShared<InLinkChangeFrame>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        tlv->setSa(stream.readMacAddress());
        tlv->setPortRole(stream.readUint16Be());
        tlv->setInID(stream.readUint16Be());
        tlv->setInterval(stream.readUint16Be());
        tlv->setLinkInfo(stream.readUint16Be());
        return tlv;
    }
    case INLINKSTATUSPOLL: {
        auto tlv = makeShared<InLinkStatusPollFrame>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        tlv->setSa(stream.readMacAddress());
        tlv->setPortRole(stream.readUint16Be());
        tlv->setInID(stream.readUint16Be());
        return tlv;
    }
    case OPTION: {
        auto tlv = makeShared<OptionHeader>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        tlv->setOuiType(static_cast<MrpOuiType>(stream.readUint24Be()));
        tlv->setEd1Type(stream.readUint8());
        return tlv;
    }
    default:
        throw cRuntimeError("Unknown Header TYPE value: %d",
                static_cast<int>(headerType));
    }
}

const Ptr<Chunk> MrpSubTlvSerializer::deserialize(MemoryInputStream &stream) const {
    auto subType = static_cast<SubTlvHeaderType>(stream.readUint8());
    switch (subType) {
    case RESERVED: {
        auto subTlv = makeShared<ManufacturerFktHeader>();
        subTlv->setSubType(subType);
        subTlv->setSubHeaderLength(stream.readUint8());
        return subTlv;
    }
    case AUTOMGR: {
        auto subTlv = makeShared<SubTlvHeader>();
        subTlv->setSubType(subType);
        subTlv->setSubHeaderLength(stream.readUint8());
        return subTlv;
    }
    case TEST_MGR_NACK:
    case TEST_PROPAGATE: {
        auto subTlv = makeShared<SubTlvTestFrame>();
        subTlv->setSubType(subType);
        subTlv->setSubHeaderLength(stream.readUint8());
        subTlv->setPrio(stream.readUint16Be());
        subTlv->setSa(stream.readMacAddress());
        subTlv->setOtherMRMPrio(stream.readUint16Be());
        subTlv->setOtherMRMSa(stream.readMacAddress());
        return subTlv;
    }
    default:
        throw cRuntimeError("Unknown Header TYPE value: %d",
                static_cast<int>(subType));
    }
}

} // namespace inet

