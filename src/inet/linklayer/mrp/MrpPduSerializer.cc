//
// Copyright (C) 2024 Daniel Zeitler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/mrp/MrpPduSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/mrp/MrpPdu_m.h"
#include "inet/linklayer/mrp/ContinuityCheckMessage_m.h"

namespace inet {

Register_Serializer(MrpTlvHeader, MrpTlvSerializer);
Register_Serializer(MrpTest, MrpTlvSerializer);
Register_Serializer(MrpTopologyChange, MrpTlvSerializer);
Register_Serializer(MrpLinkChange, MrpTlvSerializer);
Register_Serializer(MrpInLinkChange, MrpTlvSerializer);
Register_Serializer(MrpInLinkStatusPoll, MrpTlvSerializer);
Register_Serializer(MrpInTopologyChange, MrpTlvSerializer);
Register_Serializer(MrpCommon, MrpTlvSerializer);
Register_Serializer(MrpOption, MrpTlvSerializer);
Register_Serializer(MrpSubTlvHeader, MrpSubTlvSerializer);
Register_Serializer(MrpManufacturerFkt, MrpSubTlvSerializer);
Register_Serializer(MrpSubTlvTest, MrpSubTlvSerializer);
Register_Serializer(MrpVersion, MrpVersionFieldSerializer);

void MrpVersionFieldSerializer::serialize(MemoryOutputStream &stream, const Ptr<const Chunk> &chunk) const {
    const auto &version = staticPtrCast<const MrpVersion>(chunk);
    stream.writeUint16Be(version->getVersion());
}

void MrpTlvSerializer::serialize(MemoryOutputStream &stream, const Ptr<const Chunk> &chunk) const {
    const auto &tlv = staticPtrCast<const MrpTlvHeader>(chunk);
    stream.writeUint8(tlv->getHeaderType());
    stream.writeUint8(tlv->getHeaderLength());

    switch (tlv->getHeaderType()) {
    case END:
        break;

    case COMMON: {
        const auto &common = staticPtrCast<const MrpCommon>(chunk);
        stream.writeUint16Be(common->getSequenceID());
        stream.writeUint64Be(common->getUuid0());
        stream.writeUint64Be(common->getUuid1());
        break;
    }
    case TEST: {
        const auto &tf = staticPtrCast<const MrpTest>(chunk);
        stream.writeUint16Be(tf->getPrio());
        stream.writeMacAddress(tf->getSa());
        stream.writeUint16Be(tf->getPortRole());
        stream.writeUint16Be(tf->getRingState());
        stream.writeUint16Be(tf->getTransition());
        stream.writeUint32Be(tf->getTimeStamp());
        break;
    }
    case TOPOLOGYCHANGE: {
        const auto &tcf = staticPtrCast<const MrpTopologyChange>(chunk);
        stream.writeUint16Be(tcf->getPrio());
        stream.writeMacAddress(tcf->getSa());
        stream.writeUint16Be(tcf->getPortRole());
        stream.writeUint16Be(tcf->getInterval());
        break;
    }
    case LINKDOWN:
    case LINKUP: {
        const auto &lcf = staticPtrCast<const MrpLinkChange>(chunk);
        stream.writeMacAddress(lcf->getSa());
        stream.writeUint16Be(lcf->getPortRole());
        stream.writeUint16Be(lcf->getInterval());
        stream.writeUint16Be(lcf->getBlocked());
        break;
    }
    case INTEST: {
        const auto &itf = staticPtrCast<const MrpInTest>(chunk);
        stream.writeUint16Be(itf->getInID());
        stream.writeMacAddress(itf->getSa());
        stream.writeUint16Be(itf->getPortRole());
        stream.writeUint16Be(itf->getInState());
        stream.writeUint16Be(itf->getTransition());
        stream.writeUint32Be(itf->getTimeStamp());
        break;
    }
    case INTOPOLOGYCHANGE: {
        const auto &itcf = staticPtrCast<const MrpInTopologyChange>(chunk);
        stream.writeMacAddress(itcf->getSa());
        stream.writeUint16Be(itcf->getInID());
        stream.writeUint16Be(itcf->getInterval());
        break;
    }
    case INLINKDOWN:
    case INLINKUP: {
        const auto &ilc = staticPtrCast<const MrpInLinkChange>(chunk);
        stream.writeMacAddress(ilc->getSa());
        stream.writeUint16Be(ilc->getPortRole());
        stream.writeUint16Be(ilc->getInID());
        stream.writeUint16Be(ilc->getInterval());
        stream.writeUint16Be(ilc->getLinkInfo());
        break;
    }
    case INLINKSTATUSPOLL: {
        const auto &ilsp = staticPtrCast<const MrpInLinkStatusPoll>(chunk);
        stream.writeMacAddress(ilsp->getSa());
        stream.writeUint16Be(ilsp->getPortRole());
        stream.writeUint16Be(ilsp->getInID());
        break;
    }
    case OPTION: {
        const auto &oh = staticPtrCast<const MrpOption>(chunk);
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
    const auto &subTlv = staticPtrCast<const MrpSubTlvHeader>(chunk);
    stream.writeUint8(subTlv->getSubType());
    stream.writeUint8(subTlv->getSubHeaderLength());

    switch (subTlv->getSubType()) {
    case RESERVED: //fallthrough to Automgr
    case AUTOMGR:
        break;
    case TEST_MGR_NACK: //fallthrough to Test_PROPAGATE
    case TEST_PROPAGATE: {
        const auto &stf = staticPtrCast<const MrpSubTlvTest>(chunk);
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
    auto mrpVersion = makeShared<MrpVersion>();
    mrpVersion->setVersion(stream.readUint16Be());
    return mrpVersion;
}

const Ptr<Chunk> MrpTlvSerializer::deserialize(MemoryInputStream &stream) const {
    auto headerType = static_cast<TlvHeaderType>(stream.readUint16Be());
    switch (headerType) {
    case END: {
        auto tlv = makeShared<MrpTlvHeader>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        return tlv;
    }
    case COMMON: {
        auto tlv = makeShared<MrpCommon>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        tlv->setSequenceID(stream.readUint16Be());
        tlv->setUuid0(stream.readUint64Be());
        tlv->setUuid1(stream.readUint64Be());
        return tlv;
    }
    case TEST: {
        auto tlv = makeShared<MrpTest>();
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
        auto tlv = makeShared<MrpTopologyChange>();
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
        auto tlv = makeShared<MrpLinkChange>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        tlv->setSa(stream.readMacAddress());
        tlv->setPortRole(stream.readUint16Be());
        tlv->setInterval(stream.readUint16Be());
        tlv->setBlocked(stream.readUint16Be());
        return tlv;
    }
    case INTEST: {
        auto tlv = makeShared<MrpInTest>();
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
        auto tlv = makeShared<MrpInTopologyChange>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        tlv->setSa(stream.readMacAddress());
        tlv->setInID(stream.readUint16Be());
        tlv->setInterval(stream.readUint16Be());
        return tlv;
    }
    case INLINKDOWN:
    case INLINKUP: {
        auto tlv = makeShared<MrpInLinkChange>();
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
        auto tlv = makeShared<MrpInLinkStatusPoll>();
        tlv->setHeaderType(headerType);
        tlv->setHeaderLength(stream.readUint8());
        tlv->setSa(stream.readMacAddress());
        tlv->setPortRole(stream.readUint16Be());
        tlv->setInID(stream.readUint16Be());
        return tlv;
    }
    case OPTION: {
        auto tlv = makeShared<MrpOption>();
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
        auto subTlv = makeShared<MrpManufacturerFkt>();
        subTlv->setSubType(subType);
        subTlv->setSubHeaderLength(stream.readUint8());
        return subTlv;
    }
    case AUTOMGR: {
        auto subTlv = makeShared<MrpSubTlvHeader>();
        subTlv->setSubType(subType);
        subTlv->setSubHeaderLength(stream.readUint8());
        return subTlv;
    }
    case TEST_MGR_NACK:
    case TEST_PROPAGATE: {
        auto subTlv = makeShared<MrpSubTlvTest>();
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

class ContinuityCheckMessageSerializer : public FieldsChunkSerializer {
public:
    using FieldsChunkSerializer::FieldsChunkSerializer;
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;
};


void ContinuityCheckMessageSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    // ITU Y.1731 Section 9.2.2
    const auto& ccm = staticPtrCast<const ContinuityCheckMessage>(chunk);
    stream.writeUint8(ccm->getMdLevel());  // MD Level + Version
    stream.writeUint8(ccm->getOpCode());
    stream.writeUint8(ccm->getFlags());
    stream.writeUint8(70); // First TLV offset
    stream.writeUint32Be(ccm->getSequenceNumber());
    stream.writeUint16Be(ccm->getEndpointIdentifier());

    // MEG ID
    stream.writeUint8(0); // reserved
    stream.writeUint8(4); // format
    stream.writeUint8(strlen(ccm->getMessageName()));
    stream.writeBytes(reinterpret_cast<const uint8_t*>(ccm->getMessageName()), B(strlen(ccm->getMessageName())));
    stream.writeByteRepeatedly(0, 45-strlen(ccm->getMessageName()));

    stream.writeUint32Be(0); // TxFCf
    stream.writeUint32Be(0); // RxFCb
    stream.writeUint32Be(0); // TxFCb
    stream.writeUint32Be(0); // Reserved
    stream.writeUint8(0);  // End TLV
}

const Ptr<Chunk> ContinuityCheckMessageSerializer::deserialize(MemoryInputStream& stream) const
{
    auto ccm = makeShared<ContinuityCheckMessage>();
    ccm->setMdLevel(stream.readUint8());
    ccm->setOpCode(stream.readUint8());
    ccm->setFlags(stream.readUint8());
    stream.readUint8(); // First TLV offset, ignored
    ccm->setSequenceNumber(stream.readUint32Be());
    ccm->setEndpointIdentifier(stream.readUint16Be());

    stream.readUint8(); // reserved, ignored
    stream.readUint8(); // format, ignored
    int nameLength = stream.readUint8();
    ASSERT(nameLength <= 45);
    uint8_t buffer[46];
    stream.readBytes(buffer, B(nameLength));
    buffer[nameLength] = '\0';
    ccm->setMessageName(reinterpret_cast<const char*>(buffer));

    stream.readUint32Be(); // TxFCf, ignored
    stream.readUint32Be(); // RxFCb, ignored
    stream.readUint32Be(); // TxFCb, ignored
    stream.readUint32Be(); // Reserved, ignored
    stream.readUint8(); // End TLV, ignored
    return ccm;
}

Register_Serializer(ContinuityCheckMessage, ContinuityCheckMessageSerializer);


} // namespace inet

