//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QOSACKHANDLER_H
#define __INET_QOSACKHANDLER_H

#include <map>

#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"
#include "inet/linklayer/ieee80211/mac/contract/IAckHandler.h"

namespace inet {
namespace ieee80211 {

/*
 * TODO processFailedFrame
 */
// TODO this class seems to be able to handle all ACs at once, so maybe it should be moved to Hcf
class INET_API QosAckHandler : public cSimpleModule, public IAckHandler
{
  public:
    enum class Status {
        FRAME_NOT_YET_TRANSMITTED,
        NO_ACK_REQUIRED,
        BLOCK_ACK_NOT_YET_REQUESTED,
        WAITING_FOR_NORMAL_ACK,
        WAITING_FOR_BLOCK_ACK,
        NORMAL_ACK_NOT_ARRIVED,
        NORMAL_ACK_ARRIVED,
        BLOCK_ACK_ARRIVED_UNACKED,
        BLOCK_ACK_ARRIVED_ACKED,
        BLOCK_ACK_NOT_ARRIVED
    };

  protected:
    typedef std::pair<MacAddress, std::pair<Tid, SequenceControlField>> QoSKey;
    typedef std::pair<MacAddress, SequenceControlField> Key;
    std::map<QoSKey, Status> ackStatuses;
    std::map<Key, Status> mgmtAckStatuses;

  protected:
    virtual void initialize(int stage) override;

    virtual Status getQoSDataAckStatus(const QoSKey& id);
    virtual Status getMgmtOrNonQoSAckStatus(const Key& id);

    void printAckStatuses();

  public:
    virtual ~QosAckHandler() {}

    virtual void processReceivedAck(const Ptr<const Ieee80211AckFrame>& ack, const Ptr<const Ieee80211DataOrMgmtHeader>& ackedHeader);
    virtual std::set<std::pair<MacAddress, std::pair<Tid, SequenceControlField>>> processReceivedBlockAck(const Ptr<const Ieee80211BlockAck>& blockAck);
    virtual void processFailedBlockAckReq(const Ptr<const Ieee80211BlockAckReq>& blockAckReq);

    virtual void frameGotInProgress(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) override;
    virtual void processTransmittedDataOrMgmtFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& header);
    virtual void processTransmittedBlockAckReq(const Ptr<const Ieee80211BlockAckReq>& blockAckReq);
    virtual void processFailedFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader);
    virtual void dropFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader);
    virtual void dropFrames(std::set<std::pair<MacAddress, std::pair<Tid, SequenceControlField>>> seqAndFragNums);

    virtual Status getQoSDataAckStatus(const Ptr<const Ieee80211DataHeader>& header);
    virtual Status getMgmtOrNonQoSAckStatus(const Ptr<const Ieee80211DataOrMgmtHeader>& header);

    virtual bool isEligibleToTransmit(const Ptr<const Ieee80211DataOrMgmtHeader>& header) override;
    virtual bool isOutstandingFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& header) override;

    static std::string getStatusString(Status status);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

