//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/INETUtils.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceHandler.h"
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
            // TODO: check if not for us and abort
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
        frameSequence->startSequence(context, 0);
        startFrameSequenceStep();
    }
    else
        throw cRuntimeError("Channel access granted while a frame sequence is running");
}


void FrameSequenceHandler::startFrameSequenceStep()
{
    ASSERT(isSequenceRunning());
    auto nextStep = frameSequence->prepareStep(context);
    EV_INFO << "Starting next frame sequence step: history = " << frameSequence->getHistory() << "\n";
    if (nextStep == nullptr)
        finishFrameSequence();
    else {
        context->addStep(nextStep);
        switch (nextStep->getType()) {
            case IFrameSequenceStep::Type::TRANSMIT: {
                auto transmitStep = static_cast<TransmitStep *>(nextStep);
                EV_INFO << "Transmitting, frame = " << transmitStep->getFrameToTransmit() << ".\n";
                callback->transmitFrame(transmitStep->getFrameToTransmit(), transmitStep->getIfs());
                // TODO: lifetime
                // if (auto dataFrame = dynamic_cast<const Ptr<const Ieee80211DataHeader>& >(transmitStep->getFrameToTransmit()))
                //    transmitLifetimeHandler->frameTransmitted(dataFrame);
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
    callback->frameSequenceFinished();
    delete context;
    delete frameSequence;
    context = nullptr;
    frameSequence = nullptr;
    callback = nullptr;
}

void FrameSequenceHandler::abortFrameSequence()
{
    EV_INFO << "Frame sequence aborted.\n";
    auto step = context->getLastStep();
    auto failedTxStep = check_and_cast<ITransmitStep*>(dynamic_cast<IReceiveStep*>(step) ? context->getStepBeforeLast() : step);
    auto frameToTransmit = failedTxStep->getFrameToTransmit();
    auto header = frameToTransmit->peekAtFront<Ieee80211MacHeader>();
    if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header))
        callback->originatorProcessFailedFrame(frameToTransmit);
    else if (auto rtsTxStep = dynamic_cast<RtsTransmitStep *>(failedTxStep))
        callback->originatorProcessRtsProtectionFailed(const_cast<Packet *>(rtsTxStep->getProtectedFrame()));
    else if (auto blockAckReq = dynamicPtrCast<const Ieee80211BlockAckReq>(header))
        callback->originatorProcessFailedFrame(frameToTransmit);
    callback->frameSequenceFinished();
    delete context;
    delete frameSequence;
    context = nullptr;
    frameSequence = nullptr;
    callback = nullptr;
}

FrameSequenceHandler::~FrameSequenceHandler()
{
    delete frameSequence;
    delete context;
}

} // namespace ieee80211
} // namespace inet
