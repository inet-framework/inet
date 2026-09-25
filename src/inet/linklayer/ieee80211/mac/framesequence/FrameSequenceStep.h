//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FRAMESEQUENCESTEP_H
#define __INET_FRAMESEQUENCESTEP_H

#include "inet/linklayer/ieee80211/mac/contract/IFrameSequence.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"

namespace inet {
namespace ieee80211 {

class INET_API TransmitStep : public ITransmitStep
{
  protected:
    Completion completion = Completion::UNDEFINED;
    Packet *frameToTransmit = nullptr;
    const simtime_t ifs = -1;
    bool owner = false;
    const physicallayer::IIeee80211Mode *const preparedMode = nullptr;
    const b preparedLength = b(-1);

  public:
    TransmitStep(Packet *frame, simtime_t ifs, bool owner = false) :
        frameToTransmit(frame),
        ifs(ifs),
        owner(owner)
    {}

    // The caller transfers the packet only if owner is true and construction succeeds.
    TransmitStep(Packet *frame, simtime_t ifs, const physicallayer::IIeee80211Mode *mode, bool owner) :
        frameToTransmit(frame), ifs(ifs), owner(owner), preparedMode(mode), preparedLength(frame ? frame->getDataLength() : b(-1))
    {
        if (frame == nullptr || mode == nullptr || ifs < SIMTIME_ZERO)
            throw cRuntimeError("Prepared transmit step requires a mode and a nonnegative interval");
    }

    virtual ~TransmitStep() { if (owner) delete frameToTransmit; }

    virtual Type getType() override { return Type::TRANSMIT; }
    virtual const physicallayer::IIeee80211Mode *getPreparedMode() const override { return preparedMode; }
    virtual b getPreparedLength() const override { return preparedLength; }
    virtual simtime_t getPreparedInterval() const override { return ifs; }
    virtual simtime_t getPpduDuration() const override {
        if (preparedMode == nullptr)
            throw cRuntimeError("Transmit step has no prepared mode");
        return preparedMode->getDuration(preparedLength);
    }
    virtual Completion getCompletion() override { return completion; }
    virtual void setCompletion(Completion completion) override { this->completion = completion; }
    virtual Packet *getFrameToTransmit() override { return frameToTransmit; }
    virtual simtime_t getIfs() override { return ifs; }
};

class INET_API RtsTransmitStep : public TransmitStep
{
  protected:
    const Packet *protectedFrame = nullptr;

  public:
    RtsTransmitStep(Packet *protectedFrame, Packet *frame, simtime_t ifs) :
        TransmitStep(frame, ifs, true),
        protectedFrame(protectedFrame)
    {}

    RtsTransmitStep(Packet *protectedFrame, Packet *frame, simtime_t ifs, const physicallayer::IIeee80211Mode *mode) :
        TransmitStep(frame, ifs, mode, true), protectedFrame(protectedFrame)
    {}

    virtual const Packet *getProtectedFrame() { return protectedFrame; }
};

class INET_API ReceiveStep : public IReceiveStep
{
  protected:
    Completion completion = Completion::UNDEFINED;
    simtime_t timeout = -1;
    Packet *receivedFrame = nullptr;
    const physicallayer::IIeee80211Mode *const preparedMode = nullptr;
    const b preparedLength = b(-1);
    const simtime_t preparedInterval = -1;
    const Ieee80211FrameType expectedFrameType = ST_ACK;
    const MacAddress expectedPeer;

  public:
    ReceiveStep(simtime_t timeout = -1) :
        timeout(timeout)
    {}
    ReceiveStep(simtime_t timeout, Ieee80211FrameType frameType, const MacAddress& peer,
            const physicallayer::IIeee80211Mode *mode, b length, simtime_t interval) :
        timeout(timeout), preparedMode(mode), preparedLength(length), preparedInterval(interval),
        expectedFrameType(frameType), expectedPeer(peer)
    {
        if (mode == nullptr || length < b(0) || interval < SIMTIME_ZERO || timeout < SIMTIME_ZERO)
            throw cRuntimeError("Prepared receive step requires a mode and nonnegative timing and length");
    }
    virtual ~ReceiveStep() { delete receivedFrame; }

    virtual Type getType() override { return Type::RECEIVE; }
    virtual const physicallayer::IIeee80211Mode *getPreparedMode() const override { return preparedMode; }
    virtual b getPreparedLength() const override { return preparedLength; }
    virtual simtime_t getPreparedInterval() const override { return preparedInterval; }
    virtual simtime_t getPpduDuration() const override {
        if (preparedMode == nullptr)
            throw cRuntimeError("Receive step has no prepared mode");
        return preparedMode->getDuration(preparedLength);
    }
    virtual Ieee80211FrameType getExpectedFrameType() const override {
        if (preparedMode == nullptr)
            throw cRuntimeError("Receive step has no prepared frame type");
        return expectedFrameType;
    }
    virtual const MacAddress& getExpectedPeer() const override { return expectedPeer; }
    virtual Completion getCompletion() override { return completion; }
    virtual void setCompletion(Completion completion) override { this->completion = completion; }
    virtual simtime_t getTimeout() override { return timeout; }
    virtual Packet *getReceivedFrame() override { return receivedFrame; }
    virtual void setFrameToReceive(Packet *frame) override { this->receivedFrame = frame; }
};

} // namespace ieee80211
} // namespace inet

#endif
