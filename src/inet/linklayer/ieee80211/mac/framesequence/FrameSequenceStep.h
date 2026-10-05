//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FRAMESEQUENCESTEP_H
#define __INET_FRAMESEQUENCESTEP_H

#include "inet/linklayer/ieee80211/mac/contract/IFrameSequence.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequencePlan.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211RateContextTag_m.h"

namespace inet {
namespace ieee80211 {

class INET_API TransmitStep : public ITransmitStep
{
  protected:
    Completion completion = Completion::UNDEFINED;
    Packet *frameToTransmit = nullptr;
    simtime_t ifs = -1;
    bool owner = false;
    std::unique_ptr<PreparedTransmit> prepared;

  public:
    TransmitStep(Packet *frame, simtime_t ifs, bool owner = false) :
        frameToTransmit(frame),
        ifs(ifs),
        owner(owner)
    {}

    virtual ~TransmitStep() { if (owner) delete frameToTransmit; }

    virtual Completion getCompletion() override { return completion; }
    virtual void setCompletion(Completion completion) override { this->completion = completion; }
    virtual Packet *getFrameToTransmit() override { return frameToTransmit; }
    virtual simtime_t getIfs() override { return ifs; }
    [[nodiscard]] const PreparedTransmit *getPreparedTransmit() const override { return prepared.get(); }
    void setPreparedTransmit(const PreparedTransmit& value) { prepared = std::make_unique<PreparedTransmit>(value); }
    void setPreparedDuration(simtime_t duration) { prepared->duration = duration; }
};

class INET_API RtsTransmitStep : public TransmitStep
{
  protected:
    const Packet *protectedFrame = nullptr;

  public:
    RtsTransmitStep(Packet *protectedFrame, Packet *frame, simtime_t ifs) :
        TransmitStep(frame, ifs, true),
        protectedFrame(protectedFrame)
    {
        if (auto tag = protectedFrame->findTag<Ieee80211RateContextTag>())
            *frame->addTag<Ieee80211RateContextTag>() = *tag;
    }

    virtual const Packet *getProtectedFrame() { return protectedFrame; }
};

class INET_API ReceiveStep : public IReceiveStep
{
  protected:
    Completion completion = Completion::UNDEFINED;
    simtime_t timeout = -1;
    Packet *receivedFrame = nullptr;
    std::unique_ptr<PreparedReceive> prepared;

  public:
    ReceiveStep(simtime_t timeout = -1) :
        timeout(timeout)
    {}
    virtual ~ReceiveStep() { delete receivedFrame; }

    virtual Completion getCompletion() override { return completion; }
    virtual void setCompletion(Completion completion) override { this->completion = completion; }
    virtual simtime_t getTimeout() override { return timeout; }
    virtual Packet *getReceivedFrame() override { return receivedFrame; }
    virtual void setFrameToReceive(Packet *frame) override { this->receivedFrame = frame; }
    [[nodiscard]] const PreparedReceive *getPreparedReceive() const override { return prepared.get(); }
    void setPreparedReceive(const PreparedReceive& value) { prepared = std::make_unique<PreparedReceive>(value); }
};

} // namespace ieee80211
} // namespace inet

#endif
