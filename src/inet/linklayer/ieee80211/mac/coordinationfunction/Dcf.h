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

#ifndef __INET_DCF_H
#define __INET_DCF_H

#include "inet/linklayer/ieee80211/mac/channelaccess/Dcaf.h"
#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/ICoordinationFunction.h"
#include "inet/linklayer/ieee80211/mac/contract/ICtsPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/ICtsProcedure.h"
#include "inet/linklayer/ieee80211/mac/contract/IFrameSequenceHandler.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorMacDataService.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateControl.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientAckPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientAckProcedure.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientMacDataService.h"
#include "inet/linklayer/ieee80211/mac/contract/IRtsProcedure.h"
#include "inet/linklayer/ieee80211/mac/contract/IRx.h"
#include "inet/linklayer/ieee80211/mac/contract/ITx.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h"
#include "inet/linklayer/ieee80211/mac/lifetime/DcfReceiveLifetimeHandler.h"
#include "inet/linklayer/ieee80211/mac/lifetime/DcfTransmitLifetimeHandler.h"
#include "inet/linklayer/ieee80211/mac/originator/AckHandler.h"
#include "inet/linklayer/ieee80211/mac/originator/NonQosRecoveryProcedure.h"
#include "inet/linklayer/ieee80211/mac/protectionmechanism/OriginatorProtectionMechanism.h"

namespace inet {
namespace ieee80211 {

class Ieee80211Mac;

/**
 * Implements IEEE 802.11 Distributed Coordination Function.
 */
class INET_API Dcf : public ICoordinationFunction, public IFrameSequenceHandler::ICallback, public IChannelAccess::ICallback, public ITx::ICallback, public IProcedureCallback, public ModeSetListener
{
    protected:
        Ieee80211Mac *mac = nullptr;
        IRateControl *dataAndMgmtRateControl = nullptr;

        cMessage *startRxTimer = nullptr;

        // Transmission and reception
        IRx *rx = nullptr;
        ITx *tx = nullptr;

        IRateSelection *rateSelection = nullptr;

        // Channel access method
        Dcaf *channelAccess = nullptr;

        // MAC Data Service
        IOriginatorMacDataService *originatorDataService = nullptr;
        IRecipientMacDataService *recipientDataService = nullptr;

        // MAC Procedures
        AckHandler *ackHandler = nullptr;
        IOriginatorAckPolicy *originatorAckPolicy = nullptr;
        IRecipientAckProcedure *recipientAckProcedure = nullptr;
        IRecipientAckPolicy *recipientAckPolicy = nullptr;
        IRtsProcedure *rtsProcedure = nullptr;
        IRtsPolicy *rtsPolicy = nullptr;
        ICtsProcedure *ctsProcedure = nullptr;
        ICtsPolicy *ctsPolicy = nullptr;
        NonQosRecoveryProcedure *recoveryProcedure = nullptr;

        // TODO: Unimplemented
        ITransmitLifetimeHandler *transmitLifetimeHandler = nullptr;
        DcfReceiveLifetimeHandler *receiveLifetimeHandler = nullptr;

        // Protection mechanism
        OriginatorProtectionMechanism *originatorProtectionMechanism = nullptr;

        // Frame sequence handler
        IFrameSequenceHandler *frameSequenceHandler = nullptr;

        // Station counters
        StationRetryCounters *stationRetryCounters = nullptr;


    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;
        virtual void forEachChild(cVisitor *v) override;
        virtual void handleMessage(cMessage *msg) override;
        virtual void updateDisplayString();

        virtual void sendUp(const std::vector<Packet *>& completeFrames);
        virtual bool hasFrameToTransmit();
        virtual bool isReceptionInProgress();
        virtual FrameSequenceContext *buildContext();

        virtual void recipientProcessReceivedFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header);
        virtual void recipientProcessReceivedControlFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header);
        virtual void recipientProcessTransmittedControlResponseFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header);

    protected:
        // IChannelAccess::ICallback
        virtual void channelGranted(IChannelAccess *channelAccess) override;

        // IFrameSequenceHandler::ICallback
        virtual void transmitFrame(Packet *packet, simtime_t ifs) override;
        virtual void originatorProcessRtsProtectionFailed(Packet *packet) override;
        virtual void originatorProcessTransmittedFrame(Packet *packet) override;
        virtual void originatorProcessReceivedFrame(Packet *packet, Packet *lastTransmittedPacket) override;
        virtual void originatorProcessFailedFrame(Packet *packet) override;
        virtual void frameSequenceFinished() override;
        virtual void scheduleStartRxTimer(simtime_t timeout) override;

        // ITx::ICallback
        virtual void transmissionComplete(Packet *packet, const Ptr<const Ieee80211MacHeader>& header) override;

        // IProcedureCallback
       virtual void transmitControlResponseFrame(Packet *responsePacket, const Ptr<const Ieee80211MacHeader>& responseHeader, Packet *receivedPacket, const Ptr<const Ieee80211MacHeader>& receivedHeader) override;
       virtual void processMgmtFrame(Packet *mgmtPacket, const Ptr<const Ieee80211MgmtHeader>& mgmtHeader) override;

       virtual bool isSentByUs(const Ptr<const Ieee80211MacHeader>& header) const;
       virtual bool isForUs(const Ptr<const Ieee80211MacHeader>& header) const;

    public:
        virtual ~Dcf();

        // ICoordinationFunction
        virtual void processUpperFrame(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header) override;
        virtual void processLowerFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header) override;
        virtual void corruptedFrameReceived() override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_DCF_H
