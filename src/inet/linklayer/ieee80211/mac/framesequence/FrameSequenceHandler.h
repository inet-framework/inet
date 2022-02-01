//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FRAMESEQUENCEHANDLER_H
#define __INET_FRAMESEQUENCEHANDLER_H

#include "inet/linklayer/ieee80211/mac/contract/IFrameSequence.h"
#include "inet/linklayer/ieee80211/mac/contract/IFrameSequenceHandler.h"
#include "inet/linklayer/ieee80211/mac/contract/ITransmitLifetimeHandler.h"

namespace inet {
namespace ieee80211 {

class INET_API FrameSequenceHandler : public IFrameSequenceHandler
{
  protected:
    IFrameSequenceHandler::ICallback *callback = nullptr;
    IFrameSequence *frameSequence = nullptr;
    FrameSequenceContext *context = nullptr;

  protected:
    virtual void startFrameSequenceStep();
    virtual void finishFrameSequenceStep();
    virtual void finishFrameSequence();
    virtual void abortFrameSequence();

  public:
    virtual const FrameSequenceContext *getContext() const override { return context; }
    virtual const IFrameSequence *getFrameSequence() const override { return frameSequence; }
    virtual void startFrameSequence(IFrameSequence *frameSequence, FrameSequenceContext *context, IFrameSequenceHandler::ICallback *callback) override;
    virtual void processResponse(Packet *frame) override;
    virtual void transmissionComplete() override;
    virtual void handleStartRxTimeout() override;
    virtual bool isSequenceRunning() override { return frameSequence != nullptr; }

    virtual ~FrameSequenceHandler();
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

