//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
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
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"

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
        { }

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
        { }

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
        virtual int getNumSteps() const { return steps.size(); }
        virtual IFrameSequenceStep *getStep(int i) const { return steps[i]; }
        virtual IFrameSequenceStep *getLastStep() const { return steps.size() > 0 ? steps.back() : nullptr; }
        virtual IFrameSequenceStep *getStepBeforeLast() const { return steps.size() > 1 ? steps[steps.size() - 2] : nullptr; }

        virtual InProgressFrames* getInProgressFrames() const { return inProgressFrames; }
        virtual IRtsProcedure* getRtsProcedure() const { return rtsProcedure; }
        virtual IRtsPolicy* getRtsPolicy() const { return rtsPolicy; }

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
