//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/Ieee80211MacHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

namespace {

void copyBasicFields(const Ptr<ieee80211::Ieee80211MacHeader> to, const Ptr<ieee80211::Ieee80211MacHeader> from)
{
    to->setType(from->getType());
    to->setOrder(from->getOrder());
    to->setProtectedFrame(from->getProtectedFrame());
    to->setMoreData(from->getMoreData());
    to->setPowerMgmt(from->getPowerMgmt());
    to->setRetry(from->getRetry());
    to->setMoreFragments(from->getMoreFragments());
    to->setFromDS(from->getFromDS());
    to->setToDS(from->getToDS());
}

void copyActionFrameFields(const Ptr<ieee80211::Ieee80211ActionFrame> to, const Ptr<ieee80211::Ieee80211ActionFrame> from)
{
    to->setDurationField(from->getDurationField());
    to->setReceiverAddress(from->getReceiverAddress());
    to->setTransmitterAddress(from->getTransmitterAddress());
    to->setAddress3(from->getAddress3());
    to->setFragmentNumber(from->getFragmentNumber());
    to->setSequenceNumber(from->getSequenceNumber());
    to->setCategory(from->getCategory());
}

void copyBlockAckReqFrameFields(const Ptr<ieee80211::Ieee80211BlockAckReq> to, const Ptr<ieee80211::Ieee80211BlockAckReq> from)
{
    to->setDurationField(from->getDurationField());
    to->setReceiverAddress(from->getReceiverAddress());
    to->setTransmitterAddress(from->getTransmitterAddress());
    to->setBarAckPolicy(from->getBarAckPolicy());
    to->setMultiTid(from->getMultiTid());
    to->setCompressedBitmap(from->getCompressedBitmap());
    to->setReserved(from->getReserved());
}

void copyBlockAckFrameFields(const Ptr<ieee80211::Ieee80211BlockAck> to, const Ptr<ieee80211::Ieee80211BlockAck> from)
{
    to->setDurationField(from->getDurationField());
    to->setReceiverAddress(from->getReceiverAddress());
    to->setTransmitterAddress(from->getTransmitterAddress());
    to->setBlockAckPolicy(from->getBlockAckPolicy());
    to->setMultiTid(from->getMultiTid());
    to->setCompressedBitmap(from->getCompressedBitmap());
    to->setReserved(from->getReserved());
}

uint16_t packSequenceControl(uint8_t fragmentNumber, uint16_t sequenceNumber)
{
    return (fragmentNumber & 0xF) | ((sequenceNumber & 0xFFF) << 4);
}

uint16_t packBlockAckParameters(bool aMsduSupported, bool blockAckPolicy, uint8_t tid, uint16_t bufferSize)
{
    // IEEE Std 802.11-2024, 9.4.1.13, Figure 9-151.
    return aMsduSupported | (blockAckPolicy << 1) | ((tid & 0xF) << 2) | ((bufferSize & 0x3FF) << 6);
}

void writeSequenceControl(MemoryOutputStream& stream, uint8_t fragmentNumber, uint16_t sequenceNumber)
{
    stream.writeUint16Le(packSequenceControl(fragmentNumber, sequenceNumber));
}

void readSequenceControl(MemoryInputStream& stream, int& fragmentNumber, ieee80211::SequenceNumberCyclic& sequenceNumber)
{
    auto sequenceControl = stream.readUint16Le();
    fragmentNumber = sequenceControl & 0xF;
    sequenceNumber = ieee80211::SequenceNumberCyclic((sequenceControl >> 4) & 0xFFF);
}

} // namespace

namespace ieee80211 {

Register_Serializer(Ieee80211MacHeader, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211DataOrMgmtHeader, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211DataHeader, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211MgmtHeader, Ieee80211MacHeaderSerializer);

Register_Serializer(Ieee80211AckFrame, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211RtsFrame, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211CtsFrame, Ieee80211MacHeaderSerializer);

Register_Serializer(Ieee80211BasicBlockAckReq, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211CompressedBlockAckReq, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211MultiTidBlockAckReq, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211BasicBlockAck, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211CompressedBlockAck, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211MultiTidBlockAck, Ieee80211MacHeaderSerializer);

Register_Serializer(Ieee80211ActionFrame, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211AddbaRequest, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211AddbaResponse, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211Delba, Ieee80211MacHeaderSerializer);

Register_Serializer(Ieee80211MacTrailer, Ieee80211MacTrailerSerializer);

Register_Serializer(Ieee80211MsduSubframeHeader, Ieee80211MsduSubframeHeaderSerializer);
Register_Serializer(Ieee80211MpduSubframeHeader, Ieee80211MpduSubframeHeaderSerializer);

void Ieee80211MsduSubframeHeaderSerializer::serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    auto msduSubframe = dynamicPtrCast<const Ieee80211MsduSubframeHeader>(chunk);
    stream.writeMacAddress(msduSubframe->getDa());
    stream.writeMacAddress(msduSubframe->getSa());
    stream.writeUint16Be(msduSubframe->getLength());
}

const Ptr<Chunk> Ieee80211MsduSubframeHeaderSerializer::deserializeFields(MemoryInputStream& stream, const std::type_info&) const
{
    auto msduSubframe = makeShared<Ieee80211MsduSubframeHeader>();
    msduSubframe->setDa(stream.readMacAddress());
    msduSubframe->setSa(stream.readMacAddress());
    msduSubframe->setLength(stream.readUint16Be());
    return msduSubframe;
}

void Ieee80211MpduSubframeHeaderSerializer::serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    auto mpduSubframe = dynamicPtrCast<const Ieee80211MpduSubframeHeader>(chunk);
    stream.writeUint4(0);
    stream.writeUint4(mpduSubframe->getLength() >> 8);
    stream.writeUint8(mpduSubframe->getLength() & 0xFF);
    stream.writeByte(0);
    stream.writeByte(0x4E);
}

const Ptr<Chunk> Ieee80211MpduSubframeHeaderSerializer::deserializeFields(MemoryInputStream& stream, const std::type_info&) const
{
    auto mpduSubframe = makeShared<Ieee80211MpduSubframeHeader>();
    stream.readUint4();
    mpduSubframe->setLength(stream.readUint4() >> 8);
    mpduSubframe->setLength(stream.readUint8());
    stream.readByte();
    stream.readByte();
    return mpduSubframe;
}

void Ieee80211MacHeaderSerializer::serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    B startPos = stream.getLength();
    auto macHeader = dynamicPtrCast<const Ieee80211MacHeader>(chunk);
    stream.writeUint4(macHeader->getSubType());
    stream.writeUint2(macHeader->getFrameType());
    stream.writeUint2(macHeader->getProtocolVersion());
    stream.writeBit(macHeader->getOrder());
    stream.writeBit(macHeader->getProtectedFrame());
    stream.writeBit(macHeader->getMoreData());
    stream.writeBit(macHeader->getPowerMgmt());
    stream.writeBit(macHeader->getRetry());
    stream.writeBit(macHeader->getMoreFragments());
    stream.writeBit(macHeader->getFromDS());
    stream.writeBit(macHeader->getToDS());
    Ieee80211FrameType type = macHeader->getType();
    switch (type) {
        case ST_ASSOCIATIONREQUEST:
        case ST_ASSOCIATIONRESPONSE:
        case ST_REASSOCIATIONREQUEST:
        case ST_REASSOCIATIONRESPONSE:
        case ST_PROBEREQUEST:
        case ST_PROBERESPONSE:
        case ST_BEACON:
        case ST_ATIM:
        case ST_DISASSOCIATION:
        case ST_AUTHENTICATION:
        case ST_DEAUTHENTICATION:
        case ST_ACTION:
        case ST_NOACKACTION: {
            auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(chunk);
            stream.writeUint16Le(mgmtHeader->getDurationField().inUnit(SIMTIME_US));
            stream.writeMacAddress(mgmtHeader->getReceiverAddress());
            stream.writeMacAddress(mgmtHeader->getTransmitterAddress());
            stream.writeMacAddress(mgmtHeader->getAddress3());
            writeSequenceControl(stream, mgmtHeader->getFragmentNumber(), mgmtHeader->getSequenceNumber().get());
            if (mgmtHeader->getOrder())
                stream.writeUint32Be(0);
            if (type == ST_ACTION) {
                auto actionFrame = dynamicPtrCast<const Ieee80211ActionFrame>(chunk);
                switch (actionFrame->getCategory()) {
                    case 3: {
                        stream.writeByte(actionFrame->getCategory());
                        switch (actionFrame->getBlockAckAction()) {
                            case 0: {
                                auto addbaRequest = dynamicPtrCast<const Ieee80211AddbaRequest>(chunk);
                                stream.writeByte(addbaRequest->getBlockAckAction());
                                stream.writeByte(addbaRequest->getDialogToken());
                                stream.writeUint16Le(packBlockAckParameters(addbaRequest->getAMsduSupported(),
                                        addbaRequest->getBlockAckPolicy(), addbaRequest->getTid(), addbaRequest->getBufferSize()));
                                stream.writeUint16Le(addbaRequest->getBlockAckTimeoutValue().inUnit(SIMTIME_US) / 1024);
                                // IEEE Std 802.11-2024, 9.6.4.2: Fragment Number is zero.
                                writeSequenceControl(stream, 0, addbaRequest->getStartingSequenceNumber().get());
                                ASSERT(stream.getLength() - startPos == addbaRequest->getChunkLength());
                                break;
                            }
                            case 1: {
                                auto addbaResponse = dynamicPtrCast<const Ieee80211AddbaResponse>(chunk);
                                stream.writeByte(addbaResponse->getBlockAckAction());
                                stream.writeByte(addbaResponse->getDialogToken());
                                stream.writeUint16Le(addbaResponse->getStatusCode());
                                stream.writeUint16Le(packBlockAckParameters(addbaResponse->getAMsduSupported(),
                                        addbaResponse->getBlockAckPolicy(), addbaResponse->getTid(), addbaResponse->getBufferSize()));
                                stream.writeUint16Le(addbaResponse->getBlockAckTimeoutValue().inUnit(SIMTIME_US) / 1024);
                                ASSERT(stream.getLength() - startPos == addbaResponse->getChunkLength());
                                break;
                            }
                            case 2: {
                                auto delba = dynamicPtrCast<const Ieee80211Delba>(chunk);
                                stream.writeByte(delba->getBlockAckAction());
                                // IEEE Std 802.11-2024, 9.4.1.16, Figure 9-154.
                                stream.writeUint16Le((delba->getReserved() & 0x7FF) |
                                        (delba->getInitiator() << 11) | ((delba->getTid() & 0xF) << 12));
                                stream.writeUint16Le(delba->getReasonCode());
                                ASSERT(stream.getLength() - startPos == delba->getChunkLength());
                                break;
                            }
                            default:
                                throw cRuntimeError("Ieee80211MacHeaderSerializer: cannot serialize the Ieee80211ActionFrame frame, blockAckAction %d not supported.", actionFrame->getBlockAckAction());
                        }
                        break;
                    }
                    default:
                        throw cRuntimeError("Ieee80211MacHeaderSerializer: cannot serialize the Ieee80211ActionFrame frame, category %d not supported.", actionFrame->getCategory());
                }
                break;
            }
            else
                ASSERT(stream.getLength() - startPos == mgmtHeader->getChunkLength());
            break;
        }
        case ST_RTS: {
            auto rtsFrame = dynamicPtrCast<const Ieee80211RtsFrame>(chunk);
            stream.writeUint16Le(rtsFrame->getDurationField().inUnit(SIMTIME_US));
            stream.writeMacAddress(rtsFrame->getReceiverAddress());
            stream.writeMacAddress(rtsFrame->getTransmitterAddress());
            ASSERT(stream.getLength() - startPos == rtsFrame->getChunkLength());
            break;
        }
        case ST_CTS: {
            auto ctsFrame = dynamicPtrCast<const Ieee80211CtsFrame>(chunk);
            stream.writeUint16Le(ctsFrame->getDurationField().inUnit(SIMTIME_US));
            stream.writeMacAddress(ctsFrame->getReceiverAddress());
            ASSERT(stream.getLength() - startPos == ctsFrame->getChunkLength());
            break;
        }
        case ST_ACK: {
            auto ackFrame = dynamicPtrCast<const Ieee80211AckFrame>(chunk);
            stream.writeUint16Le(ackFrame->getDurationField().inUnit(SIMTIME_US));
            stream.writeMacAddress(ackFrame->getReceiverAddress());
            ASSERT(stream.getLength() - startPos == ackFrame->getChunkLength());
            break;
        }
        case ST_BLOCKACK_REQ: {
            auto blockAckReq = dynamicPtrCast<const Ieee80211BlockAckReq>(chunk);
            stream.writeUint16Le(blockAckReq->getDurationField().inUnit(SIMTIME_US));
            stream.writeMacAddress(blockAckReq->getReceiverAddress());
            stream.writeMacAddress(blockAckReq->getTransmitterAddress());
            stream.writeBit(blockAckReq->getBarAckPolicy());
            bool multiTid = blockAckReq->getMultiTid();
            bool compressedBitmap = blockAckReq->getCompressedBitmap();
            stream.writeBit(multiTid);
            stream.writeBit(compressedBitmap);
            stream.writeNBitsOfUint64Be(blockAckReq->getReserved(), 9);
            if (!multiTid && !compressedBitmap) {
                auto basicBlockAckReq = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(chunk);
                stream.writeUint4(basicBlockAckReq->getTidInfo());
                stream.writeUint32Be(basicBlockAckReq->getFragmentNumber());
                stream.writeUint64Be(0);
                stream.writeUint64Be(basicBlockAckReq->getStartingSequenceNumber().get());
                ASSERT(stream.getLength() - startPos == basicBlockAckReq->getChunkLength());
            }
            else if (!multiTid && compressedBitmap) {
                auto compressedBlockAckReq = dynamicPtrCast<const Ieee80211CompressedBlockAckReq>(chunk);
                stream.writeUint4(compressedBlockAckReq->getTidInfo());
                stream.writeUint32Be(compressedBlockAckReq->getFragmentNumber());
                stream.writeUint64Be(0);
                stream.writeUint64Be(compressedBlockAckReq->getStartingSequenceNumber().get());
                ASSERT(stream.getLength() - startPos == compressedBlockAckReq->getChunkLength());
            }
            else if (multiTid && compressedBitmap) {
                throw cRuntimeError("Ieee80211MacHeaderSerializer: cannot serialize the frame, Ieee80211MultiTidBlockAckReq unimplemented.");
            }
            else
                throw cRuntimeError("Ieee80211MacHeaderSerializer: cannot serialize the frame, multiTid = 1 && compressedBitmap = 0 is reserved.");
            break;
        }
        case ST_BLOCKACK: {
            auto blockAck = dynamicPtrCast<const Ieee80211BlockAck>(chunk);
            stream.writeUint16Le(blockAck->getDurationField().inUnit(SIMTIME_US));
            stream.writeMacAddress(blockAck->getReceiverAddress());
            stream.writeMacAddress(blockAck->getTransmitterAddress());
            stream.writeBit(blockAck->getBlockAckPolicy());
            bool multiTid = blockAck->getMultiTid();
            bool compressedBitmap = blockAck->getCompressedBitmap();
            stream.writeBit(multiTid);
            stream.writeBit(compressedBitmap);
            stream.writeNBitsOfUint64Be(blockAck->getReserved(), 9);
            if (!multiTid && !compressedBitmap) {
                auto basicBlockAck = dynamicPtrCast<const Ieee80211BasicBlockAck>(chunk);
                stream.writeUint4(basicBlockAck->getTidInfo());
                stream.writeUint16Be(basicBlockAck->getStartingSequenceNumber().get());
                for (size_t i = 0; i < 64; ++i) {
                    stream.writeByte(basicBlockAck->getBlockAckBitmap(i).getBytes()[0]);
                    stream.writeByte(basicBlockAck->getBlockAckBitmap(i).getBytes()[1]);
                }
                ASSERT(stream.getLength() - startPos == basicBlockAck->getChunkLength());
            }
            else if (!multiTid && compressedBitmap) {
                auto compressedBlockAck = dynamicPtrCast<const Ieee80211CompressedBlockAck>(chunk);
                stream.writeUint4(compressedBlockAck->getTidInfo());
                stream.writeUint16Be(compressedBlockAck->getStartingSequenceNumber().get());
                for (size_t i = 0; i < 8; ++i) {
                    stream.writeByte(compressedBlockAck->getBlockAckBitmap().getBytes()[i]);
                }
                ASSERT(stream.getLength() - startPos == compressedBlockAck->getChunkLength());
            }
            else if (multiTid && compressedBitmap) {
                throw cRuntimeError("Ieee80211MacHeaderSerializer: cannot serialize the frame, Ieee80211MultiTidBlockAck unimplemented.");
            }
            else {
                throw cRuntimeError("Ieee80211MacHeaderSerializer: cannot serialize the frame, multiTid = 1 && compressedBitmap = 0 is reserved.");
            }
            break;

        }
        case ST_DATA_WITH_QOS:
        case ST_DATA: {
            auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(chunk);
            stream.writeUint16Le(dataHeader->getDurationField().inUnit(SIMTIME_US));
            stream.writeMacAddress(dataHeader->getReceiverAddress());
            stream.writeMacAddress(dataHeader->getTransmitterAddress());
            stream.writeMacAddress(dataHeader->getAddress3());
            writeSequenceControl(stream, dataHeader->getFragmentNumber(), dataHeader->getSequenceNumber().get());
            if (dataHeader->getFromDS() && dataHeader->getToDS())
                stream.writeMacAddress(dataHeader->getAddress4());
            if (type == ST_DATA_WITH_QOS) {
                stream.writeUint4(dataHeader->getTid());
                stream.writeBit(true);
                stream.writeUint2(dataHeader->getAckPolicy());
                stream.writeBit(dataHeader->getAMsduPresent());
                stream.writeByte(0);
            }
            ASSERT(stream.getLength() - startPos == dataHeader->getChunkLength());
            break;
        }
        case ST_PSPOLL:
        case ST_LBMS_REQUEST:
        case ST_LBMS_REPORT: {
            break;
        }
        default:
            throw cRuntimeError("Ieee80211Serializer: cannot serialize the frame, type %d not supported.", type);
    }
}

const Ptr<Chunk> Ieee80211MacHeaderSerializer::deserializeFields(MemoryInputStream& stream, const std::type_info&) const
{
    auto macHeader = makeShared<Ieee80211MacHeader>();
    uint8_t subType = stream.readUint4();
    uint8_t frameType = stream.readUint2();
    uint8_t protocolVersion = stream.readUint2();
    macHeader->setType(protocolVersion, frameType, subType);
    bool order = stream.readBit();
    macHeader->setOrder(order);
    macHeader->setProtectedFrame(stream.readBit());
    macHeader->setMoreData(stream.readBit());
    macHeader->setPowerMgmt(stream.readBit());
    macHeader->setRetry(stream.readBit());
    macHeader->setMoreFragments(stream.readBit());
    macHeader->setFromDS(stream.readBit());
    macHeader->setToDS(stream.readBit());
    Ieee80211FrameType type = macHeader->getType();
    switch (type) {
        case ST_ASSOCIATIONREQUEST:
        case ST_ASSOCIATIONRESPONSE:
        case ST_REASSOCIATIONREQUEST:
        case ST_REASSOCIATIONRESPONSE:
        case ST_PROBEREQUEST:
        case ST_PROBERESPONSE:
        case ST_BEACON:
        case ST_ATIM:
        case ST_DISASSOCIATION:
        case ST_AUTHENTICATION:
        case ST_DEAUTHENTICATION:
        case ST_NOACKACTION: {
            auto mgmtHeader = makeShared<Ieee80211MgmtHeader>();
            copyBasicFields(mgmtHeader, macHeader);
            mgmtHeader->setDurationField(SimTime(stream.readUint16Le(), SIMTIME_US));
            mgmtHeader->setReceiverAddress(stream.readMacAddress());
            mgmtHeader->setTransmitterAddress(stream.readMacAddress());
            mgmtHeader->setAddress3(stream.readMacAddress());
            int fragmentNumber;
            SequenceNumberCyclic sequenceNumber;
            readSequenceControl(stream, fragmentNumber, sequenceNumber);
            mgmtHeader->setFragmentNumber(fragmentNumber);
            mgmtHeader->setSequenceNumber(sequenceNumber);
            if (order)
                stream.readUint32Be();
            return mgmtHeader;
        }
        case ST_ACTION: {
            auto actionFrame = makeShared<Ieee80211ActionFrame>();
            copyBasicFields(actionFrame, macHeader);
            actionFrame->setDurationField(SimTime(stream.readUint16Le(), SIMTIME_US));
            actionFrame->setReceiverAddress(stream.readMacAddress());
            actionFrame->setTransmitterAddress(stream.readMacAddress());
            actionFrame->setAddress3(stream.readMacAddress());
            int fragmentNumber;
            SequenceNumberCyclic sequenceNumber;
            readSequenceControl(stream, fragmentNumber, sequenceNumber);
            actionFrame->setFragmentNumber(fragmentNumber);
            actionFrame->setSequenceNumber(sequenceNumber);
            if (order)
                stream.readUint32Be();
            actionFrame->setCategory(stream.readByte());
            switch (actionFrame->getCategory()) {
                case 3: {
                    uint8_t blockAckAction = stream.readByte();
                    switch (blockAckAction) {
                        case 0: {
                            auto addbaRequest = makeShared<Ieee80211AddbaRequest>();
                            copyBasicFields(addbaRequest, macHeader);
                            copyActionFrameFields(addbaRequest, actionFrame);
                            addbaRequest->setBlockAckAction(blockAckAction);
                            addbaRequest->setDialogToken(stream.readByte());
                            auto parameters = stream.readUint16Le();
                            addbaRequest->setAMsduSupported(parameters & 1);
                            addbaRequest->setBlockAckPolicy(parameters & 2);
                            addbaRequest->setTid((parameters >> 2) & 0xF);
                            addbaRequest->setBufferSize(parameters >> 6);
                            addbaRequest->setBlockAckTimeoutValue(SimTime(stream.readUint16Le() * 1024, SIMTIME_US));
                            readSequenceControl(stream, fragmentNumber, sequenceNumber);
                            addbaRequest->set_fragmentNumber(fragmentNumber);
                            addbaRequest->setStartingSequenceNumber(sequenceNumber);
                            return addbaRequest;
                        }
                        case 1: {
                            auto addbaResponse = makeShared<Ieee80211AddbaResponse>();
                            copyBasicFields(addbaResponse, macHeader);
                            copyActionFrameFields(addbaResponse, actionFrame);
                            addbaResponse->setBlockAckAction(blockAckAction);
                            addbaResponse->setDialogToken(stream.readByte());
                            addbaResponse->setStatusCode(stream.readUint16Le());
                            auto parameters = stream.readUint16Le();
                            addbaResponse->setAMsduSupported(parameters & 1);
                            addbaResponse->setBlockAckPolicy(parameters & 2);
                            addbaResponse->setTid((parameters >> 2) & 0xF);
                            addbaResponse->setBufferSize(parameters >> 6);
                            addbaResponse->setBlockAckTimeoutValue(SimTime(stream.readUint16Le() * 1024, SIMTIME_US));
                            return addbaResponse;
                        }
                        case 2: {
                            auto delba = makeShared<Ieee80211Delba>();
                            copyBasicFields(delba, macHeader);
                            copyActionFrameFields(delba, actionFrame);
                            delba->setBlockAckAction(blockAckAction);
                            auto parameters = stream.readUint16Le();
                            delba->setReserved(parameters & 0x7FF);
                            delba->setInitiator(parameters & 0x800);
                            delba->setTid(parameters >> 12);
                            delba->setReasonCode(stream.readUint16Le());
                            return delba;
                        }
                        default:
                            actionFrame->markIncorrect();
                            return actionFrame;
                    }
                    break;
                }
                default: {
                    actionFrame->markIncorrect();
                    return actionFrame;
                }
            }
        }
        case ST_RTS: {
            auto rtsFrame = makeShared<Ieee80211RtsFrame>();
            copyBasicFields(rtsFrame, macHeader);
            rtsFrame->setDurationField(SimTime(stream.readUint16Le(), SIMTIME_US));
            rtsFrame->setReceiverAddress(stream.readMacAddress());
            rtsFrame->setTransmitterAddress(stream.readMacAddress());
            return rtsFrame;
        }
        case ST_CTS: {
            auto ctsFrame = makeShared<Ieee80211CtsFrame>();
            copyBasicFields(ctsFrame, macHeader);
            ctsFrame->setDurationField(SimTime(stream.readUint16Le(), SIMTIME_US));
            ctsFrame->setReceiverAddress(stream.readMacAddress());
            return ctsFrame;
        }
        case ST_ACK: {
            auto ackFrame = makeShared<Ieee80211AckFrame>();
            copyBasicFields(ackFrame, macHeader);
            ackFrame->setDurationField(SimTime(stream.readUint16Le(), SIMTIME_US));
            ackFrame->setReceiverAddress(stream.readMacAddress());
            return ackFrame;
        }
        case ST_BLOCKACK_REQ: {
            auto blockAckReq = makeShared<Ieee80211BlockAckReq>();
            copyBasicFields(blockAckReq, macHeader);
            blockAckReq->setDurationField(SimTime(stream.readUint16Le(), SIMTIME_US));
            blockAckReq->setReceiverAddress(stream.readMacAddress());
            blockAckReq->setTransmitterAddress(stream.readMacAddress());
            blockAckReq->setBarAckPolicy(stream.readBit());
            bool multiTid = stream.readBit();
            bool compressedBitmap = stream.readBit();
            blockAckReq->setMultiTid(multiTid);
            blockAckReq->setCompressedBitmap(compressedBitmap);
            blockAckReq->setReserved(stream.readNBitsToUint64Be(9));
            if (!multiTid && !compressedBitmap) {
                auto basicBlockAckReq = makeShared<Ieee80211BasicBlockAckReq>();
                copyBasicFields(basicBlockAckReq, macHeader);
                copyBlockAckReqFrameFields(basicBlockAckReq, blockAckReq);
                basicBlockAckReq->setTidInfo(stream.readUint4());
                basicBlockAckReq->setFragmentNumber(stream.readUint32Be());
                stream.readUint64Be();
                basicBlockAckReq->setStartingSequenceNumber(SequenceNumberCyclic(stream.readUint64Be()));
                return basicBlockAckReq;
            }
            else if (!multiTid && compressedBitmap) {
                auto compressedBlockAckReq = makeShared<Ieee80211CompressedBlockAckReq>();
                copyBasicFields(compressedBlockAckReq, macHeader);
                copyBlockAckReqFrameFields(compressedBlockAckReq, blockAckReq);
                compressedBlockAckReq->setTidInfo(stream.readUint4());
                compressedBlockAckReq->setFragmentNumber(stream.readUint32Be());
                stream.readUint64Be();
                compressedBlockAckReq->setStartingSequenceNumber(SequenceNumberCyclic(stream.readUint64Be()));
                return compressedBlockAckReq;
            }
            else
                blockAckReq->markIncorrect();
            return blockAckReq;
        }
        case ST_BLOCKACK: {
            auto blockAck = makeShared<Ieee80211BlockAck>();
            copyBasicFields(blockAck, macHeader);
            blockAck->setDurationField(SimTime(stream.readUint16Le(), SIMTIME_US));
            blockAck->setReceiverAddress(stream.readMacAddress());
            blockAck->setTransmitterAddress(stream.readMacAddress());
            blockAck->setBlockAckPolicy(stream.readBit());
            bool multiTid = stream.readBit();
            bool compressedBitmap = stream.readBit();
            blockAck->setMultiTid(multiTid);
            blockAck->setCompressedBitmap(compressedBitmap);
            blockAck->setReserved(stream.readNBitsToUint64Be(9));
            if (!multiTid && !compressedBitmap) {
                auto basicBlockAck = makeShared<Ieee80211BasicBlockAck>();
                copyBasicFields(basicBlockAck, macHeader);
                copyBlockAckFrameFields(basicBlockAck, blockAck);
                basicBlockAck->setTidInfo(stream.readUint4());
                basicBlockAck->setStartingSequenceNumber(SequenceNumberCyclic(stream.readUint16Be()));
                for (size_t i = 0; i < 64; ++i) {
                    std::vector<uint8_t> bytes;
                    bytes.push_back(stream.readByte());
                    bytes.push_back(stream.readByte());
                    BitVector *blockAckBitmap = new BitVector(bytes);
                    basicBlockAck->setBlockAckBitmap(i, *blockAckBitmap);
                }
                return basicBlockAck;
            }
            else if (!multiTid && compressedBitmap) {
                auto compressedBlockAck = makeShared<Ieee80211CompressedBlockAck>();
                copyBasicFields(compressedBlockAck, macHeader);
                copyBlockAckFrameFields(compressedBlockAck, blockAck);

                compressedBlockAck->setTidInfo(stream.readUint4());
                compressedBlockAck->setStartingSequenceNumber(SequenceNumberCyclic(stream.readUint16Be()));
                std::vector<uint8_t> bytes;
                for (size_t i = 0; i < 8; ++i) {
                    bytes.push_back(stream.readByte());
                }
                compressedBlockAck->setBlockAckBitmap(*(new BitVector(bytes)));
                return compressedBlockAck;
            }
            else {
                blockAck->markIncorrect();
                return blockAck;
            }
            return blockAck;

        }
        case ST_DATA_WITH_QOS:
        case ST_DATA: {
            auto dataHeader = makeShared<Ieee80211DataHeader>();
            copyBasicFields(dataHeader, macHeader);
            dataHeader->setDurationField(SimTime(stream.readUint16Le(), SIMTIME_US));
            dataHeader->setReceiverAddress(stream.readMacAddress());
            dataHeader->setTransmitterAddress(stream.readMacAddress());
            dataHeader->setAddress3(stream.readMacAddress());
            int fragmentNumber;
            SequenceNumberCyclic sequenceNumber;
            readSequenceControl(stream, fragmentNumber, sequenceNumber);
            dataHeader->setFragmentNumber(fragmentNumber);
            dataHeader->setSequenceNumber(sequenceNumber);
            if (dataHeader->getFromDS() && dataHeader->getToDS())
                dataHeader->setAddress4(stream.readMacAddress());
            if (type == ST_DATA_WITH_QOS) {
                dataHeader->setTid(stream.readUint4());
                stream.readBit();
                dataHeader->setAckPolicy(static_cast<AckPolicy>(stream.readUint2()));
                dataHeader->setAMsduPresent(stream.readBit());
                stream.readByte();
            }
            return dataHeader;
        }
        case ST_PSPOLL:
        case ST_LBMS_REQUEST:
        case ST_LBMS_REPORT: {
            return macHeader;
        }
        default: {
            macHeader->markIncorrect();
            return macHeader;
        }
    }
}

void Ieee80211MacTrailerSerializer::serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& macTrailer = dynamicPtrCast<const Ieee80211MacTrailer>(chunk);
    auto fcsMode = macTrailer->getFcsMode();
    if (fcsMode != FCS_COMPUTED)
        throw cRuntimeError("Cannot serialize Ieee80211FcsTrailer without properly computed FCS, try changing the value of the fcsMode parameter (e.g. in the Ieee80211Mac module)");
    stream.writeUint32Be(macTrailer->getFcs());
}

const Ptr<Chunk> Ieee80211MacTrailerSerializer::deserializeFields(MemoryInputStream& stream, const std::type_info&) const
{
    auto macTrailer = makeShared<Ieee80211MacTrailer>();
    auto fcs = stream.readUint32Be();
    macTrailer->setFcs(fcs);
    macTrailer->setFcsMode(FCS_COMPUTED);
    return macTrailer;
}

} // namespace ieee80211

} // namespace inet
