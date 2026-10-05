//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FRAMESEQUENCECONTEXT_H
#define __INET_FRAMESEQUENCECONTEXT_H

#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckProcedure.h"
#include "inet/linklayer/ieee80211/mac/contract/ICoordinationFunction.h"
#include "inet/linklayer/ieee80211/mac/contract/IFrameSequence.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorAckPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorBlockAckProcedure.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorQoSAckPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IRtsPolicy.h"
#include "inet/linklayer/ieee80211/mac/originator/RtsProcedure.h"
#include "inet/linklayer/ieee80211/mac/originator/TxopProcedure.h"
#include "inet/linklayer/ieee80211/mac/queue/InProgressFrames.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequencePlan.h"
#include "inet/linklayer/ieee80211/mac/contract/IQosRateSelection.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {
namespace ieee80211 {

class INET_API QoSContext
{
  public:
    QoSContext(IOriginatorQoSAckPolicy *ackPolicy, IOriginatorBlockAckProcedure *blockAckProcedure, IOriginatorBlockAckAgreementHandler *blockAckAgreementHandler, TxopProcedure *txopProcedure) :
        ackPolicy(ackPolicy),
        blockAckProcedure(blockAckProcedure),
        blockAckAgreementHandler(blockAckAgreementHandler),
        txopProcedure(txopProcedure)
    {}

    IOriginatorQoSAckPolicy *ackPolicy = nullptr;
    IOriginatorBlockAckProcedure *blockAckProcedure = nullptr;
    IOriginatorBlockAckAgreementHandler *blockAckAgreementHandler = nullptr;
    TxopProcedure *txopProcedure = nullptr;
};

class INET_API NonQoSContext
{
  public:
    NonQoSContext(IOriginatorAckPolicy *ackPolicy) :
        ackPolicy(ackPolicy)
    {}

    IOriginatorAckPolicy *ackPolicy = nullptr;
};

class INET_API FrameSequenceContext : public cObject
{
  protected:
    simtime_t startTime = simTime();
    MacAddress address = MacAddress::UNSPECIFIED_ADDRESS;
    physicallayer::Ieee80211ModeSet *modeSet = nullptr;
    InProgressFrames *inProgressFrames = nullptr;
    std::vector<IFrameSequenceStep *> steps;

    IRtsProcedure *rtsProcedure = nullptr;
    IRtsPolicy *rtsPolicy = nullptr;

    NonQoSContext *nonQoSContext = nullptr;
    QoSContext *qosContext = nullptr;
    IQosRateSelection *planningRateSelection = nullptr;
    std::unique_ptr<FrameSequencePlan> activePlan;
    std::unique_ptr<FrameSequencePlan> nextPlan;
    std::vector<std::unique_ptr<FrameSequencePlan>> retiredPlans;
    int activeFirstStep = 0;
    bool planValid = true;

  public:
    FrameSequenceContext(MacAddress address, physicallayer::Ieee80211ModeSet *modeSet, InProgressFrames *inProgressFrames, IRtsProcedure *rtsProcedure, IRtsPolicy *rtsPolicy, NonQoSContext *nonQosContext, QoSContext *qosContext);
    virtual ~FrameSequenceContext();

    virtual simtime_t getDuration() const { return simTime() - startTime; }

    virtual void addStep(IFrameSequenceStep *step) { steps.push_back(step); }
    virtual int getNumSteps() const { return steps.size(); }
    virtual IFrameSequenceStep *getStep(int i) const { return steps[i]; }
    virtual IFrameSequenceStep *getLastStep() const { return steps.size() > 0 ? steps.back() : nullptr; }
    virtual IFrameSequenceStep *getStepBeforeLast() const { return steps.size() > 1 ? steps[steps.size() - 2] : nullptr; }

    virtual InProgressFrames *getInProgressFrames() const { return inProgressFrames; }
    virtual IRtsProcedure *getRtsProcedure() const { return rtsProcedure; }
    virtual IRtsPolicy *getRtsPolicy() const { return rtsPolicy; }

    virtual NonQoSContext *getNonQoSContext() const { return nonQoSContext; }
    virtual QoSContext *getQoSContext() const { return qosContext; }
    [[nodiscard]] MacAddress getAddress() const { return address; }
    [[nodiscard]] physicallayer::Ieee80211ModeSet *getModeSet() const { return modeSet; }
    [[nodiscard]] virtual Packet *getFrameToTransmit() const { return inProgressFrames->getFrameToTransmit(); }
    [[nodiscard]] virtual bool hasFrameToTransmit() const { return inProgressFrames->hasInProgressFrames(); }
    [[nodiscard]] virtual bool isPlanning() const { return false; }
    virtual AckPolicy getAckPolicy();
    virtual bool isBlockAckReqNeeded();

    void enablePlanning(IQosRateSelection *rateSelection) { planningRateSelection = rateSelection; }
    [[nodiscard]] bool usesPlanning() const { return planningRateSelection != nullptr; }
    [[nodiscard]] FrameSequencePlan *getActivePlan() const { return activePlan.get(); }
    [[nodiscard]] FrameSequencePlan *getNextPlan() const { return nextPlan.get(); }
    [[nodiscard]] bool isPlanValid() const { return planValid; }
    void invalidatePlans() { planValid = false; }
    bool prepareInitialExchange(IFrameSequence *sequence);
    bool advanceExchange(IFrameSequence *sequence);
    [[nodiscard]] bool isPreparedTransmissionPermitted(const PreparedTransmit& record) const;

  protected:
    void prepareNextExchange(IFrameSequence *sequence);

  public:

    virtual simtime_t getAckTimeout(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtframe) const;
    virtual simtime_t getCtsTimeout(Packet *packet, const Ptr<const Ieee80211RtsFrame>& rtsFrame) const;
    virtual simtime_t getIfs() const;

    virtual bool isForUs(const Ptr<const Ieee80211MacHeader>& header) const;
    virtual bool isSentByUs(const Ptr<const Ieee80211MacHeader>& header) const;
};

class INET_API FrameSequenceDurationFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
    using cObjectResultFilter::receiveSignal;
};

class INET_API FrameSequenceNumPacketsFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
    using cObjectResultFilter::receiveSignal;
};

} // namespace ieee80211
} // namespace inet

#endif
