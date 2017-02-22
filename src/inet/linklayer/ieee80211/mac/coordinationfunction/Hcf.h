//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_HCF_H
#define __INET_HCF_H

#include "inet/linklayer/ieee80211/mac/channelaccess/Edca.h"
#include "inet/linklayer/ieee80211/mac/channelaccess/Hcca.h"
#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/IAckHandler.h"
#include "inet/linklayer/ieee80211/mac/contract/IBlockAckAgreementHandlerCallback.h"
#include "inet/linklayer/ieee80211/mac/contract/ICoordinationFunction.h"
#include "inet/linklayer/ieee80211/mac/contract/ICtsPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorBlockAckAgreementPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorBlockAckProcedure.h"
#include "inet/linklayer/ieee80211/mac/contract/IProcedureCallback.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientAckProcedure.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientBlockAckAgreementPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientBlockAckProcedure.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientQoSAckPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientQoSMacDataService.h"
#include "inet/linklayer/ieee80211/mac/contract/IRtsProcedure.h"
#include "inet/linklayer/ieee80211/mac/contract/ITx.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceHandler.h"
#include "inet/linklayer/ieee80211/mac/lifetime/EdcaTransmitLifetimeHandler.h"
#include "inet/linklayer/ieee80211/mac/originator/NonQoSRecoveryProcedure.h"
#include "inet/linklayer/ieee80211/mac/originator/OriginatorQoSMacDataService.h"
#include "inet/linklayer/ieee80211/mac/originator/QoSAckHandler.h"
#include "inet/linklayer/ieee80211/mac/originator/QoSRecoveryProcedure.h"
#include "inet/linklayer/ieee80211/mac/originator/TxopProcedure.h"
#include "inet/linklayer/ieee80211/mac/protectionmechanism/SingleProtectionMechanism.h"
#include "inet/linklayer/ieee80211/mac/queue/InProgressFrames.h"
#include "inet/linklayer/ieee80211/mac/recipient/CtsProcedure.h"

namespace inet {
namespace ieee80211 {

class Ieee80211Mac;

/**
 * Implements IEEE 802.11 Hybrid Coordination Function.
 */
class INET_API Hcf : public ICoordinationFunction, public IFrameSequenceHandler::ICallback, public IChannelAccess::ICallback, public ITx::ICallback, public IProcedureCallback, public IBlockAckAgreementHandlerCallback, public ModeSetListener
{
    protected:
        Ieee80211Mac *mac = nullptr;
        IRateControl *dataAndMgmtRateControl = nullptr;
        int numEdcafs = -1;

        cMessage *startRxTimer = nullptr;
        cMessage *inactivityTimer = nullptr;

        // Transmission and Reception
        IRx *rx = nullptr;
        ITx *tx = nullptr;

        IQoSRateSelection *rateSelection = nullptr;

        // Channel Access Methods
        Edca *edca = nullptr;
        Hcca *hcca = nullptr;

        // MAC Data Service
        IOriginatorMacDataService *originatorDataService = nullptr;
        IRecipientQoSMacDataService *recipientDataService = nullptr;

        // MAC Procedures
        IRecipientAckProcedure *recipientAckProcedure = nullptr;
        IOriginatorQoSAckPolicy *originatorAckPolicy = nullptr;
        IRecipientQoSAckPolicy *recipientAckPolicy = nullptr;
        IRtsProcedure *rtsProcedure = nullptr;
        IRtsPolicy *rtsPolicy = nullptr;
        ICtsProcedure *ctsProcedure = nullptr;
        ICtsPolicy *ctsPolicy = nullptr;
        IOriginatorBlockAckProcedure *originatorBlockAckProcedure = nullptr;
        IRecipientBlockAckProcedure *recipientBlockAckProcedure = nullptr;
        EdcaTransmitLifetimeHandler *lifetimeHandler = nullptr;
        std::vector<QoSRecoveryProcedure*> edcaDataRecoveryProcedures;
        NonQoSRecoveryProcedure *edcaMgmtAndNonQoSRecoveryProcedure = nullptr;

        // Block Ack Agreement Handlers
        IOriginatorBlockAckAgreementHandler *originatorBlockAckAgreementHandler = nullptr;
        IOriginatorBlockAckAgreementPolicy *originatorBlockAckAgreementPolicy = nullptr;
        IRecipientBlockAckAgreementHandler *recipientBlockAckAgreementHandler = nullptr;
        IRecipientBlockAckAgreementPolicy *recipientBlockAckAgreementPolicy = nullptr;

        // Ack handler
        std::vector<QoSAckHandler *> edcaAckHandlers;

        // Tx Opportunity
        std::vector<TxopProcedure*> edcaTxops;
        TxopProcedure *hccaTxop = nullptr;

        // Queues
        std::vector<PendingQueue *> edcaPendingQueues;
        PendingQueue *hccaPendingQueue = nullptr;
        std::vector<InProgressFrames *> edcaInProgressFrames;
        InProgressFrames *hccaInProgressFrame = nullptr;

        // Frame sequence handler
        IFrameSequenceHandler *frameSequenceHandler = nullptr;

        // Station retry counters
        std::vector<StationRetryCounters*> stationRetryCounters;

        // Protection mechanisms
        SingleProtectionMechanism *singleProtectionMechanism = nullptr;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;
        virtual void handleMessage(cMessage *msg) override;

        void startFrameSequence(AccessCategory ac);
        void handleInternalCollision(std::vector<Edcaf*> internallyCollidedEdcafs);

        void sendUp(const std::vector<Ieee80211Frame*>& completeFrames);
        FrameSequenceContext* buildContext(AccessCategory ac);
        virtual bool hasFrameToTransmit();
        virtual bool hasFrameToTransmit(AccessCategory ac);
        virtual bool isReceptionInProgress();

        // Recipient
        virtual void recipientProcessReceivedFrame(Ieee80211Frame *frame);
        virtual void recipientProcessReceivedControlFrame(Ieee80211Frame *frame);
        virtual void recipientProcessReceivedManagementFrame(Ieee80211ManagementFrame *frame);
        virtual void recipientProcessTransmittedControlResponseFrame(Ieee80211Frame *frame);

        // Originator
        virtual void originatorProcessTransmittedManagementFrame(Ieee80211ManagementFrame *mgmtFrame, AccessCategory ac);
        virtual void originatorProcessTransmittedControlFrame(Ieee80211Frame *controlFrame, AccessCategory ac);
        virtual void originatorProcessTransmittedDataFrame(Ieee80211DataFrame *dataFrame, AccessCategory ac);
        virtual void originatorProcessReceivedManagementFrame(Ieee80211ManagementFrame *frame, Ieee80211Frame *lastTransmittedFrame, AccessCategory ac);
        virtual void originatorProcessReceivedControlFrame(Ieee80211Frame *frame, Ieee80211Frame *lastTransmittedFrame, AccessCategory ac);
        virtual void originatorProcessReceivedDataFrame(Ieee80211DataFrame* frame, Ieee80211Frame* lastTransmittedFrame, AccessCategory ac);

        virtual void setFrameMode(Ieee80211Frame *frame, const IIeee80211Mode *mode) const;
        virtual bool isSentByUs(Ieee80211Frame *frame) const;
        virtual bool isForUs(Ieee80211Frame *frame) const;

    protected:
        // IFrameSequenceHandler::ICallback
        virtual void originatorProcessRtsProtectionFailed(Ieee80211DataOrMgmtFrame *protectedFrame) override;
        virtual void originatorProcessTransmittedFrame(Ieee80211Frame* transmittedFrame) override;
        virtual void originatorProcessReceivedFrame(Ieee80211Frame *frame, Ieee80211Frame *lastTransmittedFrame) override;
        virtual void originatorProcessFailedFrame(Ieee80211DataOrMgmtFrame* failedFrame) override;
        virtual void frameSequenceFinished() override;
        virtual void transmitFrame(Ieee80211Frame *frame, simtime_t ifs) override;
        virtual void scheduleStartRxTimer(simtime_t timeout) override;

        // IChannelAccess::ICallback
        virtual void channelGranted(IChannelAccess *channelAccess) override;

        // ITx::ICallback
        virtual void transmissionComplete(Ieee80211Frame *frame) override;

        // IProcedureCallback
        virtual void transmitControlResponseFrame(Ieee80211Frame* responseFrame, Ieee80211Frame* receivedFrame) override;
        virtual void processMgmtFrame(Ieee80211ManagementFrame *mgmtFrame) override;

        // IProcedureCallback
        virtual void scheduleInactivityTimer(simtime_t timeout) override;

    public:
        virtual ~Hcf();

        // ICoordinationFunction
        virtual void processUpperFrame(Ieee80211DataOrMgmtFrame *frame) override;
        virtual void processLowerFrame(Ieee80211Frame *frame) override;
        virtual void corruptedFrameReceived() override;

};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_HCF_H
