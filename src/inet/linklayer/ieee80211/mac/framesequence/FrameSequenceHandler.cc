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
    ASSERT(callback != nullptr);
    auto lastStep = context->getLastStep();
    switch (lastStep->getType()) {
        case IFrameSequenceStep::Type::RECEIVE: {
            // TODO check if not for us and abort
            auto receiveStep = check_and_cast<IReceiveStep *>(context->getLastStep());
            receiveStep->setFrameToReceive(frame);
            finishFrameSequenceStep();
            if (isSequenceRunning())
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
    if (isSequenceRunning()) {
        finishFrameSequenceStep();
        if (isSequenceRunning())
            startFrameSequenceStep();
    }
}

void FrameSequenceHandler::startFrameSequence(IFrameSequence *frameSequence, FrameSequenceContext *context, IFrameSequenceHandler::ICallback *callback)
{
    EV_INFO << "Starting frame sequence.\n";
    this->callback = callback;
    if (!isSequenceRunning()) {
        this->frameSequence = frameSequence;
        this->context = context;
        ++callbackDepth;
        frameSequence->startSequence(context, 0);
        --callbackDepth;
        if (cancellationRequested)
            finishFrameSequence();
        else
            startFrameSequenceStep();
    }
    else
        throw cRuntimeError("Channel access granted while a frame sequence is running");
}

void FrameSequenceHandler::startFrameSequenceStep()
{
    ASSERT(isSequenceRunning());
    ++callbackDepth;
    auto nextStep = frameSequence->prepareStep(context);
    --callbackDepth;
    if (cancellationRequested) {
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
                ++callbackDepth;
                callback->transmitFrame(transmitStep->getFrameToTransmit(), transmitStep->getIfs());
                --callbackDepth;
                if (cancellationRequested) {
                    cancellationRequested = false;
                    finishFrameSequence();
                }
                // TODO lifetime
//                if (auto dataFrame = dynamic_cast<const Ptr<const Ieee80211DataHeader>& >(transmitStep->getFrameToTransmit()))
//                    transmitLifetimeHandler->frameTransmitted(dataFrame);
                break;
            }
            case IFrameSequenceStep::Type::RECEIVE: {
                // start reception timer, break loop if timer expires before reception is over
                auto receiveStep = static_cast<IReceiveStep *>(nextStep);
                ++callbackDepth;
                callback->scheduleStartRxTimer(receiveStep->getTimeout());
                --callbackDepth;
                if (cancellationRequested)
                    finishFrameSequence();
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
    ++callbackDepth;
    auto stepResult = frameSequence->completeStep(context);
    --callbackDepth;
    if (cancellationRequested) {
        finishFrameSequence();
        return;
    }
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
                ++callbackDepth;
                callback->originatorProcessTransmittedFrame(transmitStep->getFrameToTransmit());
                --callbackDepth;
                break;
            }
            case IFrameSequenceStep::Type::RECEIVE: {
                auto receiveStep = static_cast<IReceiveStep *>(lastStep);
                auto transmitStep = check_and_cast<ITransmitStep *>(context->getStepBeforeLast());
                ++callbackDepth;
                callback->originatorProcessReceivedFrame(receiveStep->getReceivedFrame(), transmitStep->getFrameToTransmit());
                --callbackDepth;
                break;
            }
            default:
                throw cRuntimeError("Unknown frame sequence step type");
        }
        if (cancellationRequested)
            finishFrameSequence();
    }
}

void FrameSequenceHandler::finishFrameSequence()
{
    EV_INFO << "Frame sequence finished.\n";
    finishing = true;
    cancellationRequested = false;
    if (context->getOutcome() == FrameSequenceOutcome::RUNNING)
        context->setOutcome(FrameSequenceOutcome::COMPLETED);
    auto inProgressFrames = context->getInProgressFrames();
    callback->frameSequenceFinished();
    delete context;
    delete frameSequence;
    context = nullptr;
    frameSequence = nullptr;
    callback = nullptr;
    finishing = false;
    inProgressFrames->clearDroppedFrames();
}

void FrameSequenceHandler::abortFrameSequence()
{
    EV_INFO << "Frame sequence aborted.\n";
    context->setOutcome(FrameSequenceOutcome::RESPONSE_FAILED);
    auto step = context->getLastStep();
    auto failedTxStep = check_and_cast<ITransmitStep *>(dynamic_cast<IReceiveStep *>(step) ? context->getStepBeforeLast() : step);
    auto frameToTransmit = failedTxStep->getFrameToTransmit();
    auto header = frameToTransmit->peekAtFront<Ieee80211MacHeader>();
    ++callbackDepth;
    if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header))
        callback->originatorProcessFailedFrame(frameToTransmit);
    else if (auto rtsTxStep = dynamic_cast<RtsTransmitStep *>(failedTxStep))
        callback->originatorProcessRtsProtectionFailed(const_cast<Packet *>(rtsTxStep->getProtectedFrame()));
    else if (auto blockAckReq = dynamicPtrCast<const Ieee80211BlockAckReq>(header))
        callback->originatorProcessFailedFrame(frameToTransmit);
    --callbackDepth;
    finishFrameSequence();
}

FrameSequenceHandler::~FrameSequenceHandler()
{
    delete frameSequence;
    delete context;
}

void FrameSequenceHandler::recordTransmission()
{
    context->recordTransmission();
}

void FrameSequenceHandler::cancelFrameSequence(FrameSequenceOutcome outcome)
{
    if (!isSequenceRunning() || finishing)
        return;
    context->setOutcome(outcome);
    if (callbackDepth != 0)
        cancellationRequested = true;
    else
        finishFrameSequence();
}

} // namespace ieee80211
} // namespace inet
