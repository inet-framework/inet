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
#include "inet/linklayer/ieee80211/mac/contract/IAckHandler.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorMacDataService.h"
#include "inet/linklayer/ieee80211/mac/lifetime/EdcaTransmitLifetimeHandler.h"
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
    EdcaTransmitLifetimeHandler *lifetimeHandler = nullptr; // Borrowed from EDCA; null for DCF.
    std::vector<Packet *> inProgressFrames;
    std::vector<Packet *> droppedFrames;
    std::function<bool(Packet *)> transmissionEligible;

  protected:
    virtual void initialize(int stage) override;

    void ensureHasFrameToTransmit();
    bool hasEligibleFrameToTransmit();
    bool isEligibleToTransmit(Packet *packet);

  public:
    virtual ~InProgressFrames();
    void setLifetimeHandler(EdcaTransmitLifetimeHandler *handler) { lifetimeHandler = handler; }
    void setTransmissionEligibility(std::function<bool(Packet *)> predicate) { transmissionEligible = predicate; }
    bool hasEligibleInProgressFrames() { return hasEligibleFrameToTransmit(); }

    virtual std::string str() const override;
    virtual void forEachChild(cVisitor *v) override;
    virtual int getLength() const { return inProgressFrames.size(); }
    virtual Packet *getFrames(int i) const { return inProgressFrames[i]; }
    bool containsFrame(const Packet *packet) const;
    // Takes ownership after the data service commits a reversible preparation.
    void adoptPreparedFrame(Packet *packet);
    virtual Packet *getFrameToTransmit();
    virtual Packet *peekFrameToTransmit();
    SequenceNumberCyclic getCandidateSequenceNumber(Packet *packet) const;
    virtual Packet *getPendingFrameFor(Packet *frame, bool extract = true);
    virtual void dropFrame(Packet *packet);
    virtual void dropFrames(std::set<std::pair<MacAddress, std::pair<Tid, SequenceControlField>>> seqAndFragNums);

    virtual bool hasInProgressFrames() { ensureHasFrameToTransmit(); return hasEligibleFrameToTransmit(); }
    virtual std::vector<Packet *> getOutstandingFrames();

    virtual void clearDroppedFrames();
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif
