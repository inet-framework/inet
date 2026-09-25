//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IFRAMESEQUENCE_H
#define __INET_IFRAMESEQUENCE_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace physicallayer {
class IIeee80211Mode;
}
namespace ieee80211 {

class FrameSequenceContext;

enum class FrameSequenceOutcome { RUNNING = 0, COMPLETED = 1, RESPONSE_FAILED = 2, RATE_STATE_CHANGED = 3, STOPPED = 4 };

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
    // A null mode identifies a legacy step without prepared airtime information.
    // Prepared values stay fixed through execution; no query transfers ownership.
    virtual const physicallayer::IIeee80211Mode *getPreparedMode() const = 0;
    virtual b getPreparedLength() const = 0;
    virtual simtime_t getPreparedInterval() const = 0;
    // Throws if the step has no prepared mode.
    virtual simtime_t getPpduDuration() const = 0;
};

class INET_API ITransmitStep : public IFrameSequenceStep
{
  public:
    virtual Type getType() override = 0;

    virtual Packet *getFrameToTransmit() = 0;
    virtual simtime_t getIfs() = 0;
};

class INET_API IReceiveStep : public IFrameSequenceStep
{
  public:
    virtual Type getType() override = 0;

    virtual simtime_t getTimeout() = 0;
    virtual Packet *getReceivedFrame() = 0;
    virtual void setFrameToReceive(Packet *frame) = 0;
    // Throws if the receive step has no prepared response description.
    virtual Ieee80211FrameType getExpectedFrameType() const = 0;
    virtual const MacAddress& getExpectedPeer() const = 0;
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
