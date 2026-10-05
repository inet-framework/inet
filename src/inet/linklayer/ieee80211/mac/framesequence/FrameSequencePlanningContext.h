// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef INET_FRAMESEQUENCEPLANNINGCONTEXT_H
#define INET_FRAMESEQUENCEPLANNINGCONTEXT_H

#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceStep.h"

namespace inet::ieee80211 {

class INET_API FrameSequencePlanningContext : public FrameSequenceContext
{
  protected:
    std::vector<StagedFrameView> frames;
    size_t position = 0;
    int offset = 0;
    bool continuation = false;
    bool ackPolicySelected = false;
    AckPolicy selectedAckPolicy = NORMAL_ACK;
    IQosRateSelection *rateSelection;
    TransmitStep *lastTransmit = nullptr;
    std::set<int64_t> repeatedCandidates;

  public:
    FrameSequencePlanningContext(const FrameSequenceContext& source, IQosRateSelection *rateSelection, bool continuation);
    [[nodiscard]] bool isPlanning() const override { return true; }
    [[nodiscard]] Packet *getFrameToTransmit() const override;
    [[nodiscard]] bool hasFrameToTransmit() const override;
    [[nodiscard]] simtime_t getIfs() const override;
    AckPolicy getAckPolicy() override;
    bool isBlockAckReqNeeded() override { return false; }
    [[nodiscard]] const StagedFrameView *getCandidate() const;
    [[nodiscard]] bool supportsPreparation() const { return qosContext && qosContext->ackPolicy && qosContext->txopProcedure && rateSelection && rtsPolicy && rtsProcedure; }
    bool enterRepetition() { return getCandidate() && repeatedCandidates.insert(getCandidate()->identity).second; }
    void projectCompletion();
    [[nodiscard]] std::unique_ptr<FrameSequencePlan> makePlan(const IFrameSequence *sequence) const;
    void addTransmit(FrameSequencePlan& plan, bool rts);
    void addReceive(FrameSequencePlan& plan, bool cts);
};

} // namespace inet::ieee80211

#endif
