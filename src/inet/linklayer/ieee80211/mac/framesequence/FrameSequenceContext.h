//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FRAMESEQUENCECONTEXT_H
#define __INET_FRAMESEQUENCECONTEXT_H

#include <functional>
#include <memory>

#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckProcedure.h"
#include "inet/linklayer/ieee80211/mac/contract/BlockAckVariant.h"
#include "inet/linklayer/ieee80211/mac/contract/ICoordinationFunction.h"
#include "inet/linklayer/ieee80211/mac/contract/IFrameSequence.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorAckPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorBlockAckProcedure.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorQoSAckPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorQosMacDataService.h"
#include "inet/linklayer/ieee80211/mac/contract/IRtsPolicy.h"
#include "inet/linklayer/ieee80211/mac/originator/RtsProcedure.h"
#include "inet/linklayer/ieee80211/mac/originator/TxopProcedure.h"
#include "inet/linklayer/ieee80211/mac/queue/InProgressFrames.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {
namespace ieee80211 {

class FrameSequenceContext;

enum class DataPreparationResult { LEGACY, READY, DEFERRED };

/** Retains one actual mode decision until DATA submission or cancellation. */
struct INET_API PreparedDataTransmission : public cObject
{
    std::unique_ptr<PreparedQosData> preparation;
    Packet *frame = nullptr; // Borrowed from preparation, or from InProgressFrames after commitment.
    const physicallayer::Ieee80211ModeSet *modeSet = nullptr;
    const physicallayer::IIeee80211Mode *dataMode = nullptr;
    const physicallayer::IIeee80211Mode *rtsMode = nullptr;
    const physicallayer::IIeee80211Mode *ackMode = nullptr;
    const physicallayer::IIeee80211Mode *ctsMode = nullptr;
    std::unique_ptr<Packet> rtsPrediction;
    OriginatorBlockAckAgreement *agreement = nullptr; // Identity only; validation obtains current state from its owner.
    bool cancelled = false;

    ~PreparedDataTransmission() override;
    void setRtsPrediction(Packet *packet);
};

class INET_API QoSContext
{
  public:
    QoSContext(IOriginatorQoSAckPolicy *ackPolicy, IOriginatorBlockAckProcedure *blockAckProcedure, IOriginatorBlockAckAgreementHandler *blockAckAgreementHandler, TxopProcedure *txopProcedure, BlockAckVariant variant = BlockAckVariant::BASIC) :
        ackPolicy(ackPolicy),
        blockAckProcedure(blockAckProcedure),
        blockAckAgreementHandler(blockAckAgreementHandler),
        txopProcedure(txopProcedure),
        blockAckVariant(variant)
    {}

    IOriginatorQoSAckPolicy *ackPolicy = nullptr;
    IOriginatorBlockAckProcedure *blockAckProcedure = nullptr;
    IOriginatorBlockAckAgreementHandler *blockAckAgreementHandler = nullptr;
    TxopProcedure *txopProcedure = nullptr;
    const BlockAckVariant blockAckVariant;
    std::function<DataPreparationResult(FrameSequenceContext *)> prepareData;
    std::unique_ptr<PreparedDataTransmission> preparedData;
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

  public:
    FrameSequenceContext(MacAddress address, physicallayer::Ieee80211ModeSet *modeSet, InProgressFrames *inProgressFrames, IRtsProcedure *rtsProcedure, IRtsPolicy *rtsPolicy, NonQoSContext *nonQosContext, QoSContext *qosContext);
    virtual ~FrameSequenceContext();

    virtual simtime_t getDuration() const { return simTime() - startTime; }

    virtual void addStep(IFrameSequenceStep *step) { steps.push_back(step); }
    void removeLastStep();
    virtual int getNumSteps() const { return steps.size(); }
    virtual IFrameSequenceStep *getStep(int i) const { return steps[i]; }
    virtual IFrameSequenceStep *getLastStep() const { return steps.size() > 0 ? steps.back() : nullptr; }
    virtual IFrameSequenceStep *getStepBeforeLast() const { return steps.size() > 1 ? steps[steps.size() - 2] : nullptr; }

    virtual InProgressFrames *getInProgressFrames() const { return inProgressFrames; }
    // A reversible preview replaces destructive extraction only for the scoped QoS path.
    Packet *getDataFrameToTransmit() const;
    virtual IRtsProcedure *getRtsProcedure() const { return rtsProcedure; }
    virtual IRtsPolicy *getRtsPolicy() const { return rtsPolicy; }

    virtual NonQoSContext *getNonQoSContext() const { return nonQoSContext; }
    virtual QoSContext *getQoSContext() const { return qosContext; }

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
