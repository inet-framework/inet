//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IFRAMESEQUENCE_H
#define __INET_IFRAMESEQUENCE_H

#include "inet/common/packet/Packet.h"

namespace inet {
namespace ieee80211 {

class FrameSequenceContext;

class INET_API IFrameSequenceStep
{
  public:
    enum class Type {
        TRANSMIT,
        RECEIVE,
    };

    enum class Completion {
        UNDEFINED,
        ACCEPTED,
        REJECTED,
        EXPIRED,
    };

  public:
    virtual ~IFrameSequenceStep() {}

    virtual Type getType() = 0;
    virtual Completion getCompletion() = 0;
    virtual void setCompletion(Completion completion) = 0;
};

class INET_API ITransmitStep : public IFrameSequenceStep
{
  public:
    virtual Type getType() override { return Type::TRANSMIT; }

    virtual Packet *getFrameToTransmit() = 0;
    // Returns the frame that originated this transmit step. For an ordinary
    // transmit this is the frame being sent; an RTS step overrides it with
    // the frame protected by the RTS/CTS exchange.
    virtual const Packet *getOriginatingFrame() { return getFrameToTransmit(); }
    // Keep the protected-frame terminology available to existing callers
    // while exposing the same read-only contract through ITransmitStep.
    virtual const Packet *getProtectedFrame() { return getOriginatingFrame(); }
    virtual simtime_t getIfs() = 0;
};

class INET_API IReceiveStep : public IFrameSequenceStep
{
  public:
    virtual Type getType() override { return Type::RECEIVE; }

    virtual simtime_t getTimeout() = 0;
    virtual Packet *getReceivedFrame() = 0;
    virtual void setFrameToReceive(Packet *frame) = 0;
};

class INET_API IFrameSequence
{
  public:
    virtual ~IFrameSequence() {}

    virtual void startSequence(FrameSequenceContext *context, int step) = 0;
    virtual IFrameSequenceStep *prepareStep(FrameSequenceContext *context) = 0;
    virtual bool completeStep(FrameSequenceContext *context) = 0;

    virtual std::string getHistory() const = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
