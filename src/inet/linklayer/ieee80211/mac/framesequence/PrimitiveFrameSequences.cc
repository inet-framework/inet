//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/framesequence/PrimitiveFrameSequences.h"

namespace inet {
namespace ieee80211 {

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
            auto dataOrMgmtPacket = context->getInProgressFrames()->getFrameToTransmit();
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
            auto packet = context->getInProgressFrames()->getFrameToTransmit();
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
            auto packet = context->getInProgressFrames()->getFrameToTransmit();
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
            auto packet = context->getInProgressFrames()->getFrameToTransmit();
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
            auto packet = context->getInProgressFrames()->getFrameToTransmit();
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
            auto blockAckReqParams = context->getQoSContext()->ackPolicy->computeBlockAckReqParameters(context->getInProgressFrames(), context->getQoSContext()->txopProcedure);
            auto receiverAddr = std::get<0>(blockAckReqParams);
            auto startingSequenceNumber = std::get<1>(blockAckReqParams);
            auto tid = std::get<2>(blockAckReqParams);
            auto blockAckReq = context->getQoSContext()->blockAckProcedure->buildBasicBlockAckReqFrame(receiverAddr, tid, startingSequenceNumber);
            auto blockAckPacket = new Packet("BasicBlockAckReq", blockAckReq);
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
            return context->isForUs(receivedHeader) && receivedHeader->getType() == ST_BLOCKACK;
        }
        default:
            throw cRuntimeError("Unknown step");
    }
}

} // namespace ieee80211
} // namespace inet

