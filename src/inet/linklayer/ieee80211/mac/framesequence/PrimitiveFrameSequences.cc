//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/framesequence/PrimitiveFrameSequences.h"

namespace inet {
namespace ieee80211 {

bool isValidCompressedBlockAck(Packet *packet, MacAddress local, MacAddress peer, Tid tid)
{
    auto response = dynamicPtrCast<const Ieee80211CompressedBlockAck>(packet->peekAtFront<Ieee80211MacHeader>());
    return response && response->getReceiverAddress() == local && response->getTransmitterAddress() == peer &&
        response->getTidInfo() == tid && !response->getMultiTid() && response->getCompressedBitmap() &&
        response->getFragmentNumber() == 0 && response->getStartingSequenceNumber().isSpecified() &&
        response->getBlockAckBitmap().getSize() == 64 &&
        response->getChunkLength() == LENGTH_COMPRESSED_BLOCKACK - B(4) && packet->getDataLength() == LENGTH_COMPRESSED_BLOCKACK;
}

IFrameSequenceStep *AmpduBlockAckFs::prepareStep(FrameSequenceContext *context)
{
    auto prepared = context->getQoSContext()->preparedData.get();
    if (step == 0)
        return new AmpduTransmitStep(context->getQoSContext()->preparedData, context->getIfs());
    if (step == 1)
        return new ReceiveStep(context->getQoSContext()->ackPolicy->getImplicitBlockAckTimeout(prepared->ackMode));
    return nullptr;
}

bool AmpduBlockAckFs::completeStep(FrameSequenceContext *context)
{
    if (step++ == 0)
        return true;
    auto prepared = context->getQoSContext()->preparedData.get();
    auto receive = check_and_cast<IReceiveStep *>(context->getLastStep());
    return !prepared->cancelled && isValidCompressedBlockAck(receive->getReceivedFrame(),
        prepared->limits.transmitter, prepared->limits.receiver, prepared->limits.tid);
}

// TODO remove isForUs checks it's already done in framesequencehandler

void SelfCtsFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
}

IFrameSequenceStep *SelfCtsFs::prepareStep(FrameSequenceContext *context)
{
    // TODO Implement
    return nullptr;
}

bool SelfCtsFs::completeStep(FrameSequenceContext *context)
{
    // TODO Implement
    return false;
}

void RtsFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
}

IFrameSequenceStep *RtsFs::prepareStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0: {
            auto dataOrMgmtPacket = context->getDataFrameToTransmit();
            auto rtsFrame = context->getRtsProcedure()->buildRtsFrame(dataOrMgmtPacket->peekAtFront<Ieee80211DataOrMgmtHeader>());
            auto rtsPacket = new Packet("RTS");
            rtsPacket->insertAtBack(rtsFrame);
            rtsPacket->insertAtBack(makeShared<Ieee80211MacTrailer>());
            return new RtsTransmitStep(dataOrMgmtPacket, rtsPacket, context->getIfs());
        }
        case 1:
            return nullptr;
        default:
            throw cRuntimeError("Unknown step");
    }
}

bool RtsFs::completeStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0:
            step++;
            return true;
        default:
            throw cRuntimeError("Unknown step");
    }
}

void CtsFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
}

IFrameSequenceStep *CtsFs::prepareStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0: {
            auto txStep = check_and_cast<RtsTransmitStep *>(context->getLastStep());
            auto rtsPacket = txStep->getFrameToTransmit();
            return new ReceiveStep(context->getCtsTimeout(rtsPacket, rtsPacket->peekAtFront<Ieee80211RtsFrame>()));
        }
        case 1:
            return nullptr;
        default:
            throw cRuntimeError("Unknown step");
    }
}

bool CtsFs::completeStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0: {
            auto receiveStep = check_and_cast<IReceiveStep *>(context->getStep(firstStep + step));
            step++;
            auto receivedPacket = receiveStep->getReceivedFrame();
            const auto& receivedHeader = receivedPacket->peekAtFront<Ieee80211MacHeader>();
            return context->isForUs(receivedHeader) && receivedHeader->getType() == ST_CTS;
        }
        default:
            throw cRuntimeError("Unknown step");
    }
}

void DataFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
}

IFrameSequenceStep *DataFs::prepareStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0: {
            Packet *packet = nullptr;
            if (context->getQoSContext() && dynamic_cast<IReceiveStep *>(context->getLastStep())) {
                if (auto rts = dynamic_cast<RtsTransmitStep *>(context->getStepBeforeLast()))
                    packet = const_cast<Packet *>(rts->getProtectedFrame());
            }
            // RTS/CTS retains its selected DATA even if its lifetime expires during protection.
            // HCF rejects the next submission before Tx accepts that expired DATA.
            if (packet == nullptr)
                packet = context->getDataFrameToTransmit();
            return new TransmitStep(packet, context->getIfs());
        }
        case 1:
            return nullptr;
        default:
            throw cRuntimeError("Unknown step");
    }
}

bool DataFs::completeStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0:
            step++;
            return true;
        default:
            throw cRuntimeError("Unknown step");
    }
}

void ManagementAckFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
}

IFrameSequenceStep *ManagementAckFs::prepareStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0: {
            Packet *packet = nullptr;
            if (context->getQoSContext() && dynamic_cast<IReceiveStep *>(context->getLastStep())) {
                if (auto rts = dynamic_cast<RtsTransmitStep *>(context->getStepBeforeLast()))
                    packet = const_cast<Packet *>(rts->getProtectedFrame());
            }
            if (packet == nullptr)
                packet = context->getInProgressFrames()->getFrameToTransmit();
            return new TransmitStep(packet, context->getIfs());
        }
        case 1: {
            auto txStep = check_and_cast<TransmitStep *>(context->getLastStep());
            auto packet = txStep->getFrameToTransmit();
            auto mgmtHeader = packet->peekAtFront<Ieee80211MgmtHeader>();
            return new ReceiveStep(context->getAckTimeout(packet, mgmtHeader));
        }
        case 2:
            return nullptr;

        default:
            throw cRuntimeError("Unknown step");
    }
}

bool ManagementAckFs::completeStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0:
            step++;
            return true;
        case 1: {
            auto receiveStep = check_and_cast<IReceiveStep *>(context->getStep(firstStep + step));
            step++;
            auto receivedPacket = receiveStep->getReceivedFrame();
            const auto& receivedHeader = receivedPacket->peekAtFront<Ieee80211MacHeader>();
            return context->isForUs(receivedHeader) && receivedHeader->getType() == ST_ACK;
        }
        default:
            throw cRuntimeError("Unknown step");
    }
}

void ManagementFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
}

IFrameSequenceStep *ManagementFs::prepareStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0: {
            Packet *packet = nullptr;
            if (context->getQoSContext() && dynamic_cast<IReceiveStep *>(context->getLastStep())) {
                if (auto rts = dynamic_cast<RtsTransmitStep *>(context->getStepBeforeLast()))
                    packet = const_cast<Packet *>(rts->getProtectedFrame());
            }
            if (packet == nullptr)
                packet = context->getInProgressFrames()->getFrameToTransmit();
            return new TransmitStep(packet, context->getIfs());
        }
        case 1:
            return nullptr;
        default:
            throw cRuntimeError("Unknown step");
    }
}

bool ManagementFs::completeStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0:
            step++;
            return true;
        default:
            throw cRuntimeError("Unknown step");
    }
}

void AckFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
}

IFrameSequenceStep *AckFs::prepareStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0: {
            auto txStep = check_and_cast<TransmitStep *>(context->getLastStep());
            auto packet = txStep->getFrameToTransmit();
            auto dataOrMgmtHeader = packet->peekAtFront<Ieee80211DataOrMgmtHeader>();
            return new ReceiveStep(context->getAckTimeout(packet, dataOrMgmtHeader));
        }
        case 1:
            return nullptr;
        default:
            throw cRuntimeError("Unknown step");
    }
}

bool AckFs::completeStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0: {
            auto receiveStep = check_and_cast<IReceiveStep *>(context->getStep(firstStep + step));
            step++;
            auto receivedPacket = receiveStep->getReceivedFrame();
            const auto& receivedHeader = receivedPacket->peekAtFront<Ieee80211MacHeader>();
            return context->isForUs(receivedHeader) && receivedHeader->getType() == ST_ACK;
        }
        default:
            throw cRuntimeError("Unknown step");
    }
}

void RtsCtsFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
}

IFrameSequenceStep *RtsCtsFs::prepareStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0: {
            auto packet = context->getDataFrameToTransmit();
            auto dataOrMgmtHeader = packet->peekAtFront<Ieee80211DataOrMgmtHeader>();
            auto rtsFrame = context->getRtsProcedure()->buildRtsFrame(dataOrMgmtHeader);
            auto rtsPacket = new Packet("RTS");
            rtsPacket->insertAtBack(rtsFrame);
            rtsPacket->insertAtBack(makeShared<Ieee80211MacTrailer>());
            return new RtsTransmitStep(packet, rtsPacket, context->getIfs());
        }
        case 1: {
            auto txStep = check_and_cast<RtsTransmitStep *>(context->getLastStep());
            auto packet = txStep->getFrameToTransmit();
            auto rtsFrame = packet->peekAtFront<Ieee80211RtsFrame>();
            return new ReceiveStep(context->getCtsTimeout(packet, rtsFrame));
        }
        case 2:
            return nullptr;
        default:
            throw cRuntimeError("Unknown step");
    }
}

bool RtsCtsFs::completeStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0:
            step++;
            return true;
        case 1: {
            auto receiveStep = check_and_cast<IReceiveStep *>(context->getStep(firstStep + step));
            step++;
            auto receivedPacket = receiveStep->getReceivedFrame();
            const auto& receivedHeader = receivedPacket->peekAtFront<Ieee80211MacHeader>();
            return context->isForUs(receivedHeader) && receivedHeader->getType() == ST_CTS;
        }
        default:
            throw cRuntimeError("Unknown step");
    }
}

void FragFrameAckFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
}

IFrameSequenceStep *FragFrameAckFs::prepareStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0: {
            auto frame = context->getInProgressFrames()->getFrameToTransmit();
            return new TransmitStep(frame, context->getIfs());
        }
        case 1: {
            auto txStep = check_and_cast<TransmitStep *>(context->getLastStep());
            auto packet = txStep->getFrameToTransmit();
            auto dataOrMgmtHeader = packet->peekAtFront<Ieee80211DataOrMgmtHeader>();
            return new ReceiveStep(context->getAckTimeout(packet, dataOrMgmtHeader));
        }
        case 2:
            return nullptr;
        default:
            throw cRuntimeError("Unknown step");
    }
}

bool FragFrameAckFs::completeStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0:
            step++;
            return true;
        case 1: {
            auto receiveStep = check_and_cast<IReceiveStep *>(context->getStep(firstStep + step));
            step++;
            auto receivedPacket = receiveStep->getReceivedFrame();
            const auto& receivedHeader = receivedPacket->peekAtFront<Ieee80211MacHeader>();
            return context->isForUs(receivedHeader) && receivedHeader->getType() == ST_ACK;
        }
        default:
            throw cRuntimeError("Unknown step");
    }
}

void LastFrameAckFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
}

IFrameSequenceStep *LastFrameAckFs::prepareStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0: {
            auto frame = context->getInProgressFrames()->getFrameToTransmit();
            return new TransmitStep(frame, context->getIfs());
        }
        case 1: {
            auto txStep = check_and_cast<TransmitStep *>(context->getLastStep());
            auto packet = txStep->getFrameToTransmit();
            auto dataOrMgmtHeader = packet->peekAtFront<Ieee80211DataOrMgmtHeader>();
            return new ReceiveStep(context->getAckTimeout(packet, dataOrMgmtHeader));
        }
        case 2:
            return nullptr;
        default:
            throw cRuntimeError("Unknown step");
    }
}

bool LastFrameAckFs::completeStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0:
            step++;
            return true;
        case 1: {
            auto receiveStep = check_and_cast<IReceiveStep *>(context->getStep(firstStep + step));
            step++;
            auto receivedPacket = receiveStep->getReceivedFrame();
            const auto& receivedHeader = receivedPacket->peekAtFront<Ieee80211MacHeader>();
            return context->isForUs(receivedHeader) && receivedHeader->getType() == ST_ACK;
        }
        default:
            throw cRuntimeError("Unknown step");
    }
}

void BlockAckReqBlockAckFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
}

IFrameSequenceStep *BlockAckReqBlockAckFs::prepareStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0: {
            auto qos = context->getQoSContext();
            auto blockAckReqParams = qos->ackPolicy->computeBlockAckReqParameters(context->getInProgressFrames(), qos->txopProcedure, qos->blockAckAgreementHandler);
            auto receiverAddr = std::get<0>(blockAckReqParams);
            auto startingSequenceNumber = std::get<1>(blockAckReqParams);
            auto tid = std::get<2>(blockAckReqParams);
            bool compressed = qos->blockAckVariant == BlockAckVariant::COMPRESSED;
            auto blockAckReq = compressed ? qos->blockAckProcedure->buildCompressedBlockAckReqFrame(receiverAddr, tid, startingSequenceNumber) :
                qos->blockAckProcedure->buildBasicBlockAckReqFrame(receiverAddr, tid, startingSequenceNumber);
            auto blockAckPacket = new Packet(compressed ? "CompressedBlockAckReq" : "BasicBlockAckReq", blockAckReq);
            blockAckPacket->insertAtBack(makeShared<Ieee80211MacTrailer>());
            return new TransmitStep(blockAckPacket, context->getIfs(), true);
        }
        case 1: {
            auto txStep = check_and_cast<ITransmitStep *>(context->getLastStep());
            auto packet = txStep->getFrameToTransmit();
            auto blockAckReq = packet->peekAtFront<Ieee80211BlockAckReq>();
            return new ReceiveStep(context->getQoSContext()->ackPolicy->getBlockAckTimeout(packet, blockAckReq));
        }
        case 2:
            return nullptr;
        default:
            throw cRuntimeError("Unknown step");
    }
}

bool BlockAckReqBlockAckFs::completeStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0:
            step++;
            return true;
        case 1: {
            auto receiveStep = check_and_cast<IReceiveStep *>(context->getStep(firstStep + step));
            step++;
            auto receivedPacket = receiveStep->getReceivedFrame();
            const auto& receivedHeader = receivedPacket->peekAtFront<Ieee80211MacHeader>();
            auto requestStep = check_and_cast<ITransmitStep *>(context->getStep(firstStep));
            auto request = requestStep->getFrameToTransmit()->peekAtFront<Ieee80211BlockAckReq>();
            if (auto basic = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(request)) {
                auto response = dynamicPtrCast<const Ieee80211BasicBlockAck>(receivedHeader);
                bool valid = response && !response->getMultiTid() && !response->getCompressedBitmap() &&
                    response->getFragmentNumber() == 0 && response->getStartingSequenceNumber().isSpecified() &&
                    (response->getReserved() & 3) == 0 && response->getChunkLength() == LENGTH_BASIC_BLOCKACK - B(4) &&
                    receivedPacket->getDataLength() == LENGTH_BASIC_BLOCKACK && response->getBlockAckBitmapArraySize() == 64;
                if (valid)
                    for (int i = 0; i < 64; i++)
                        valid &= response->getBlockAckBitmap(i).getSize() == 16;
                return valid && context->isForUs(receivedHeader) &&
                    response->getReceiverAddress() == basic->getTransmitterAddress() &&
                    response->getTransmitterAddress() == basic->getReceiverAddress() &&
                    response->getTidInfo() == basic->getTidInfo();
            }
            auto compressed = dynamicPtrCast<const Ieee80211CompressedBlockAckReq>(request);
            auto response = dynamicPtrCast<const Ieee80211CompressedBlockAck>(receivedHeader);
            // IEEE Std 802.11-2024, 9.3.1.8.2: this profile supports the 64-bit bitmap without dynamic fragmentation.
            return compressed && isValidCompressedBlockAck(receivedPacket, compressed->getTransmitterAddress(),
                compressed->getReceiverAddress(), compressed->getTidInfo());
        }
        default:
            throw cRuntimeError("Unknown step");
    }
}

} // namespace ieee80211
} // namespace inet

