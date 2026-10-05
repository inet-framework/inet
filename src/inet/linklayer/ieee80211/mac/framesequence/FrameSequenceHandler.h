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
    bool running = false;
    bool terminating = false;
    int callbackDepth = 0;
    uint64_t generation = 0;
    TxRequestId pendingRequest;
    struct RetiredSequence {
        FrameSequenceContext *context;
        IFrameSequence *sequence;
    };
    std::vector<RetiredSequence> retired;
    struct CallGuard {
        FrameSequenceHandler& handler;
        CallGuard(FrameSequenceHandler& handler) : handler(handler) { handler.beginCallback(); }
        ~CallGuard() noexcept(false) { handler.endCallback(); }
    };
    void disposeRetired();

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
    bool isSequenceRunning() override { return running; }
    void setPendingTransmission(TxRequestId id) override { pendingRequest = id; }
    void pendingTransmissionCanceled(TxRequestId id) override;
    void resetForLifecycle(bool onAir) override;
    void beginCallback() override { callbackDepth++; }
    void endCallback() override { ASSERT(callbackDepth > 0); if (--callbackDepth == 0) disposeRetired(); }

    virtual ~FrameSequenceHandler();
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif
