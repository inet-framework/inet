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
//

#ifndef __INET_EDCAF_H
#define __INET_EDCAF_H

#include "inet/linklayer/ieee80211/mac/common/AccessCategory.h"
#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/common/StationRetryCounters.h"
#include "inet/linklayer/ieee80211/mac/contract/IChannelAccess.h"
#include "inet/linklayer/ieee80211/mac/contract/IContention.h"
#include "inet/linklayer/ieee80211/mac/contract/IEdcaCollisionController.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecoveryProcedure.h"
#include "inet/linklayer/ieee80211/mac/contract/IRx.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/originator/QosAckHandler.h"
#include "inet/linklayer/ieee80211/mac/originator/QosRecoveryProcedure.h"
#include "inet/linklayer/ieee80211/mac/originator/TxopProcedure.h"
#include "inet/linklayer/ieee80211/mac/queue/InProgressFrames.h"

namespace inet {
namespace ieee80211 {

/**
 * Implements IEEE 802.11 Enhanced Distributed Channel Access Function.
 */
class INET_API Edcaf : public IChannelAccess, public IContention::ICallback, public IRecoveryProcedure::ICwCalculator, public ModeSetListener
{
    protected:
        IContention *contention = nullptr;
        IChannelAccess::ICallback *callback = nullptr;
        IEdcaCollisionController *collisionController = nullptr;

        StationRetryCounters *stationRetryCounters = nullptr;
        QosAckHandler *ackHandler = nullptr;
        QosRecoveryProcedure *recoveryProcedure = nullptr;

        // Tx Opportunity
        TxopProcedure *txopProcedure = nullptr;

        // Queues
        queueing::IPacketQueue *pendingQueue = nullptr;
        InProgressFrames *inProgressFrames = nullptr;

        bool owning = false;

        simtime_t slotTime = -1;
        simtime_t sifs = -1;
        simtime_t ifs = -1;
        simtime_t eifs = -1;

        AccessCategory ac = AccessCategory(-1);
        int cw = -1;
        int cwMin = -1;
        int cwMax = -1;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;
        virtual void refreshDisplay() const override;
        virtual void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details) override;

        virtual AccessCategory getAccessCategory(const char *ac);
        virtual int getAifsNumber(AccessCategory ac);

        virtual int getCwMax(AccessCategory ac, int aCwMax, int aCwMin);
        virtual int getCwMin(AccessCategory ac, int aCwMin);

        virtual void calculateTimingParameters();

    public:
        virtual ~Edcaf();

        virtual StationRetryCounters *getStationRetryCounters() const { return stationRetryCounters; }
        virtual QosAckHandler *getAckHandler() const { return ackHandler; }
        virtual QosRecoveryProcedure *getRecoveryProcedure() const { return recoveryProcedure; }

        virtual TxopProcedure *getTxopProcedure() const { return txopProcedure; }

        virtual queueing::IPacketQueue *getPendingQueue() const { return pendingQueue; }
        virtual InProgressFrames *getInProgressFrames() const { return inProgressFrames; }

        // IChannelAccess
        virtual void requestChannel(IChannelAccess::ICallback *callback) override;
        virtual void releaseChannel(IChannelAccess::ICallback *callback) override;

        // IContention::ICallback
        virtual void channelAccessGranted() override;
        virtual void expectedChannelAccess(simtime_t time) override;

        // IRecoveryProcedure::ICallback
        virtual void incrementCw() override;
        virtual void resetCw() override;
        virtual int getCw() override { return cw; }

        // Edcaf
        virtual bool isOwning() { return owning; }
        virtual bool isInternalCollision();
        virtual AccessCategory getAccessCategory() { return ac; }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_EDCAF_H
