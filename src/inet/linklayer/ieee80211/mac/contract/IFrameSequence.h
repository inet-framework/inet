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

