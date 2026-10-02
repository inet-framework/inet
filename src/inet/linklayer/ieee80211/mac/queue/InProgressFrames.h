//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INPROGRESSFRAMES_H
#define __INET_INPROGRESSFRAMES_H

#include "inet/common/SimpleModule.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"
#include "inet/linklayer/ieee80211/mac/common/TxopExchangePlan.h"
#include "inet/linklayer/ieee80211/mac/contract/IAckHandler.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorMacDataService.h"
#include "inet/queueing/contract/IPacketQueue.h"

namespace inet {
namespace ieee80211 {

class INET_API InProgressFrames : public SimpleModule
{
  public:
    static simsignal_t packetEnqueuedSignal;
    static simsignal_t packetDequeuedSignal;

  protected:
    queueing::IPacketQueue *pendingQueue = nullptr;
    IOriginatorMacDataService *dataService = nullptr;
    IAckHandler *ackHandler = nullptr;
    std::vector<Packet *> inProgressFrames;
    std::vector<Packet *> droppedFrames;
    struct FragmentHistory {
        std::map<int, b> transmittedLengths;
        int fragmentCount = 0;
        bool retried = false;
    };
    std::map<TxopFrameIdentity, FragmentHistory> fragmentHistory;

  protected:
    virtual void initialize(int stage) override;

    void ensureHasFrameToTransmit();
    bool hasEligibleFrameToTransmit();

  public:
    virtual ~InProgressFrames();

    virtual std::string str() const override;
    virtual void forEachChild(cVisitor *v) override;
    virtual int getLength() const { return inProgressFrames.size(); }
    virtual Packet *getFrames(int i) const { return inProgressFrames[i]; }
    virtual Packet *getFrameToTransmit();
    // Preparation may extract packets. The subsequent query is read-only.
    virtual void prepareCandidate(const Packet *excluded = nullptr);
    virtual Packet *findPreparedCandidate(const Packet *excluded = nullptr) const;
    virtual void describeTransmission(TxopExchangePlan& plan) const;
    virtual void recordTransmission(Packet *packet);
    static TxopFrameIdentity getFrameIdentity(const Ptr<const Ieee80211DataOrMgmtHeader>& header);
    virtual Packet *getPendingFrameFor(Packet *frame);
    virtual void dropFrame(Packet *packet);
    virtual void dropFrames(std::set<std::pair<MacAddress, std::pair<Tid, SequenceControlField>>> seqAndFragNums);

    virtual bool hasInProgressFrames() { ensureHasFrameToTransmit(); return hasEligibleFrameToTransmit(); }
    virtual std::vector<Packet *> getOutstandingFrames();

    virtual void clearDroppedFrames();
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif
