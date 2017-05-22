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

#ifndef __INET_INPROGRESSFRAMES_H
#define __INET_INPROGRESSFRAMES_H

#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"
#include "inet/linklayer/ieee80211/mac/contract/IAckHandler.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorMacDataService.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/queue/Ieee80211Queue.h"

namespace inet {
namespace ieee80211 {

class INET_API InProgressFrames
{
    protected:
        PendingQueue *pendingQueue = nullptr;
        IOriginatorMacDataService *dataService = nullptr;
        IAckHandler *ackHandler = nullptr;
        std::list<Ieee80211DataOrMgmtFrame *> inProgressFrames;
        std::vector<Ieee80211DataOrMgmtFrame *> droppedFrames;

    protected:
        void ensureHasFrameToTransmit();
        bool hasEligibleFrameToTransmit();

    public:
        InProgressFrames(PendingQueue *pendingQueue, IOriginatorMacDataService *dataService, IAckHandler *ackHandler) :
            pendingQueue(pendingQueue),
            dataService(dataService),
            ackHandler(ackHandler)
        { }
        virtual ~InProgressFrames();

        virtual Ieee80211DataOrMgmtFrame *getFrameToTransmit();
        virtual Ieee80211DataOrMgmtFrame *getPendingFrameFor(Ieee80211Frame *frame);
        virtual void dropFrame(Ieee80211DataOrMgmtFrame *dataOrMgmtFrame);
        virtual void dropFrames(std::set<std::pair<MACAddress, std::pair<Tid, SequenceControlField>>> seqAndFragNums);

        virtual bool hasInProgressFrames() { ensureHasFrameToTransmit(); return hasEligibleFrameToTransmit(); }
        virtual std::vector<Ieee80211DataFrame*> getOutstandingFrames();

        virtual void clearDroppedFrames();
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_INPROGRESSFRAMES_H
