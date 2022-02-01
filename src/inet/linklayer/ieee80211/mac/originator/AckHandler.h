//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
    virtual ~AckHandler() {}

    virtual void processReceivedAck(const Ptr<const Ieee80211AckFrame>& ack, const Ptr<const Ieee80211DataOrMgmtHeader>& ackedHeader);

    virtual void frameGotInProgress(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) override; // TODO rename
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

#endif

