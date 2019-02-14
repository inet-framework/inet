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

#ifndef __INET_ACKHANDLER_H
#define __INET_ACKHANDLER_H

#include <map>

#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"
#include "inet/linklayer/ieee80211/mac/contract/IAckHandler.h"

namespace inet {
namespace ieee80211 {

class INET_API AckHandler : public cSimpleModule, public IAckHandler
{
    public:
        enum class Status {
            FRAME_NOT_YET_TRANSMITTED,
            NO_ACK_REQUIRED,
            WAITING_FOR_ACK,
            ACK_NOT_ARRIVED,
            ACK_ARRIVED,
        };
    protected:
        std::map<SequenceControlField, Status> ackStatuses;

    protected:
        virtual void initialize(int stage) override;
        virtual Status getAckStatus(SequenceControlField id);
        void printAckStatuses();

    public:
        virtual ~AckHandler() { }

        virtual void processReceivedAck(const Ptr<const Ieee80211AckFrame>& ack, const Ptr<const Ieee80211DataOrMgmtHeader>& ackedHeader);

        virtual void frameGotInProgress(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) override; // TODO: rename
        virtual void processTransmittedDataOrMgmtFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& header);

        virtual Status getAckStatus(const Ptr<const Ieee80211DataOrMgmtHeader>& header);
        virtual bool isEligibleToTransmit(const Ptr<const Ieee80211DataOrMgmtHeader>& header) override;
        virtual bool isOutstandingFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& header) override;
        virtual void processFailedFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader);
        virtual void dropFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader);

        static std::string getStatusString(Status status);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // __INET_ACKHANDLER_H
