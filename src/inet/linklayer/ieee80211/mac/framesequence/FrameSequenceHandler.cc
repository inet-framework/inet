//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceHandler.h"

#include "inet/common/INETUtils.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceStep.h"

namespace inet {
namespace ieee80211 {

void FrameSequenceHandler::handleStartRxTimeout()
{
    CallGuard guard(*this);
    if (!running)
        return;
    auto lastStep = context->getLastStep();
    switch (lastStep->getType()) {
        case IFrameSequenceStep::Type::RECEIVE:
            abortFrameSequence();
            break;
        case IFrameSequenceStep::Type::TRANSMIT:
            throw cRuntimeError("Received timeout while in transmit step");
        default:
            throw cRuntimeError("Unknown step type");
    }
}

void FrameSequenceHandler::processResponse(Packet *frame)
{
    CallGuard guard(*this);
    auto currentGeneration = generation;
    ASSERT(callback != nullptr);
    auto lastStep = context->getLastStep();
    switch (lastStep->getType()) {
        case IFrameSequenceStep::Type::RECEIVE: {
            // TODO check if not for us and abort
            auto receiveStep = check_and_cast<IReceiveStep *>(context->getLastStep());
            receiveStep->setFrameToReceive(frame);
            finishFrameSequenceStep();
            if (isSequenceRunning() && generation == currentGeneration)
                startFrameSequenceStep();
            break;
        }
        case IFrameSequenceStep::Type::TRANSMIT:
            throw cRuntimeError("Received frame while current step is transmit");
        default:
            throw cRuntimeError("Unknown step type");
    }
}

void FrameSequenceHandler::transmissionComplete()
{
    CallGuard guard(*this);
    auto currentGeneration = generation;
    pendingRequest = {};
    if (isSequenceRunning()) {
        finishFrameSequenceStep();
        if (isSequenceRunning() && generation == currentGeneration)
            startFrameSequenceStep();
    }
}

void FrameSequenceHandler::startFrameSequence(IFrameSequence *frameSequence, FrameSequenceContext *context, IFrameSequenceHandler::ICallback *callback)
{
    CallGuard guard(*this);
    EV_INFO << "Starting frame sequence.\n";
    this->callback = callback;
    if (!isSequenceRunning()) {
        this->frameSequence = frameSequence;
        this->context = context;
        running = true;
        terminating = false;
        generation++;
        if (context->usesPlanning()) {
            if (!context->prepareInitialExchange(frameSequence)) {
                finishFrameSequence();
                return;
            }
        }
        else
            frameSequence->startSequence(context, 0);
        auto currentGeneration = generation;
        callback->frameSequenceStarted();
        if (running && generation == currentGeneration)
            startFrameSequenceStep();
    }
    else
        throw cRuntimeError("Channel access granted while a frame sequence is running");
}

void FrameSequenceHandler::startFrameSequenceStep()
{
    ASSERT(isSequenceRunning());
    auto currentGeneration = generation;
    auto nextStep = frameSequence->prepareStep(context);
    if (!running || generation != currentGeneration)
        return;
    if (nextStep == nullptr && context->usesPlanning() && context->advanceExchange(frameSequence))
        nextStep = frameSequence->prepareStep(context);
    if (!running || generation != currentGeneration)
        return;
    // An on-air request retains its response wait. Invalidation prevents future transmissions.
    if (nextStep && nextStep->getType() == IFrameSequenceStep::Type::TRANSMIT && context->usesPlanning() && !context->isPlanValid()) {
        finishFrameSequence();
        return;
    }
    EV_INFO << "Starting next frame sequence step: history = " << frameSequence->getHistory() << "\n";
    if (nextStep == nullptr)
        finishFrameSequence();
    else {
        context->addStep(nextStep);
        switch (nextStep->getType()) {
            case IFrameSequenceStep::Type::TRANSMIT: {
                auto transmitStep = static_cast<TransmitStep *>(nextStep);
                EV_INFO << "Transmitting, frame = " << transmitStep->getFrameToTransmit() << ".\n";
                callback->transmitFrame(transmitStep->getFrameToTransmit(), transmitStep->getIfs(), transmitStep->getPreparedTransmit());
                // TODO lifetime
//                if (auto dataFrame = dynamic_cast<const Ptr<const Ieee80211DataHeader>& >(transmitStep->getFrameToTransmit()))
//                    transmitLifetimeHandler->frameTransmitted(dataFrame);
                break;
            }
            case IFrameSequenceStep::Type::RECEIVE: {
                // start reception timer, break loop if timer expires before reception is over
                auto receiveStep = static_cast<IReceiveStep *>(nextStep);
                callback->scheduleStartRxTimer(receiveStep->getTimeout());
                break;
            }
            default:
                throw cRuntimeError("Unknown frame sequence step type");
        }
    }
}

void FrameSequenceHandler::finishFrameSequenceStep()
{
    ASSERT(isSequenceRunning());
    auto lastStep = context->getLastStep();
    auto stepResult = frameSequence->completeStep(context);
    EV_INFO << "Finishing last frame sequence step: history = " << frameSequence->getHistory() << "\n";
    if (!stepResult) {
        lastStep->setCompletion(IFrameSequenceStep::Completion::REJECTED);
        abortFrameSequence();
    }
    else {
        lastStep->setCompletion(IFrameSequenceStep::Completion::ACCEPTED);
        switch (lastStep->getType()) {
            case IFrameSequenceStep::Type::TRANSMIT: {
                auto transmitStep = static_cast<ITransmitStep *>(lastStep);
                callback->originatorProcessTransmittedFrame(transmitStep->getFrameToTransmit());
                break;
            }
            case IFrameSequenceStep::Type::RECEIVE: {
                auto receiveStep = static_cast<IReceiveStep *>(lastStep);
                auto transmitStep = check_and_cast<ITransmitStep *>(context->getStepBeforeLast());
                callback->originatorProcessReceivedFrame(receiveStep->getReceivedFrame(), transmitStep->getFrameToTransmit());
                break;
            }
            default:
                throw cRuntimeError("Unknown frame sequence step type");
        }
    }
}

void FrameSequenceHandler::finishFrameSequence()
{
    EV_INFO << "Frame sequence finished.\n";
    auto oldContext = context;
    auto oldSequence = frameSequence;
    running = false;
    generation++;
    context->invalidatePlans();
    callback->frameSequenceFinished();
    retired.push_back({oldContext, oldSequence});
    if (context == oldContext) {
        context = nullptr;
        frameSequence = nullptr;
        callback = nullptr;
        pendingRequest = {};
    }
}

void FrameSequenceHandler::abortFrameSequence()
{
    if (terminating)
        return;
    terminating = true;
    context->invalidatePlans();
    auto currentGeneration = generation;
    EV_INFO << "Frame sequence aborted.\n";
    auto step = context->getLastStep();
    auto failedTxStep = check_and_cast<ITransmitStep *>(dynamic_cast<IReceiveStep *>(step) ? context->getStepBeforeLast() : step);
    auto frameToTransmit = failedTxStep->getFrameToTransmit();
    auto header = frameToTransmit->peekAtFront<Ieee80211MacHeader>();
    if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header))
        callback->originatorProcessFailedFrame(frameToTransmit);
    else if (auto rtsTxStep = dynamic_cast<RtsTransmitStep *>(failedTxStep))
        callback->originatorProcessRtsProtectionFailed(const_cast<Packet *>(rtsTxStep->getProtectedFrame()));
    else if (auto blockAckReq = dynamicPtrCast<const Ieee80211BlockAckReq>(header))
        callback->originatorProcessFailedFrame(frameToTransmit);
    if (running && generation == currentGeneration)
        finishFrameSequence();
}

void FrameSequenceHandler::disposeRetired()
{
    auto disposing = std::move(retired);
    retired.clear();
    for (auto& item : disposing) {
        auto frames = item.context->getInProgressFrames();
        delete item.context;
        delete item.sequence;
        // A replacement sequence can still borrow frames from this owner.
        if (!context || context->getInProgressFrames() != frames)
            frames->clearDroppedFrames();
    }
}

void FrameSequenceHandler::pendingTransmissionCanceled(TxRequestId id)
{
    CallGuard guard(*this);
    if (!running || id != pendingRequest)
        return;
    pendingRequest = {};
    context->invalidatePlans();
    finishFrameSequence();
}

void FrameSequenceHandler::resetForLifecycle(bool onAir)
{
    CallGuard guard(*this);
    if (!running || terminating)
        return;
    context->invalidatePlans();
    auto last = context->getLastStep();
    auto currentGeneration = generation;
    if (onAir && last && last->getType() == IFrameSequenceStep::Type::TRANSMIT) {
        // Reject recursive lifecycle cleanup until actual transmission state is committed.
        terminating = true;
        auto frame = static_cast<ITransmitStep *>(last)->getFrameToTransmit();
        auto header = frame->peekAtFront<Ieee80211MacHeader>();
        bool noImmediateResponse = header->getReceiverAddress().isMulticast();
        if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header))
            noImmediateResponse |= dataHeader->getAckPolicy() == BLOCK_ACK || dataHeader->getAckPolicy() == NO_ACK;
        callback->originatorProcessTransmittedFrame(frame);
        if (!running || generation != currentGeneration)
            return;
        terminating = false;
        if (noImmediateResponse) {
            // These transmissions do not enter a Normal ACK wait or its failure transition.
            finishFrameSequence();
            return;
        }
    }
    if (last && (onAir || last->getType() == IFrameSequenceStep::Type::RECEIVE))
        abortFrameSequence();
    else
        finishFrameSequence();
}

FrameSequenceHandler::~FrameSequenceHandler()
{
    for (auto& item : retired) {
        delete item.context;
        delete item.sequence;
    }
    delete frameSequence;
    delete context;
}

} // namespace ieee80211
} // namespace inet
