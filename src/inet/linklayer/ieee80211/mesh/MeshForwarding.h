// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_MESHFORWARDING_H
#define __INET_MESHFORWARDING_H

#include <list>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshDataMac.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshForwarding.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshPathSelection.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"

namespace inet {
namespace ieee80211 {

class INET_API MeshForwarding : public OperationalBase, public IMeshForwarding
{
  protected:
    ModuleRefByPar<IMeshDataMac> mac;
    ModuleRefByPar<Ieee80211Mib> mib;
    ModuleRefByPar<IMeshPathSelection> pathSelection;
    std::map<std::pair<MacAddress, uint32_t>, simtime_t> recentFrames;
    uint32_t nextSequence = 0;
    int initialTtl = 0;
    int cacheCapacity = 0;
    simtime_t cacheLifetime;
    struct Pending { Packet *packet; MacAddress destination; simtime_t expiry; };
    std::list<Pending> pending;
    b pendingBytes = b(0);
    int maxPendingPackets = 0;
    b maxPendingBytes;
    simtime_t maxPendingWait;
    cMessage *pendingExpiryTimer = nullptr;

    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    bool remember(MacAddress source, uint32_t sequence);
    std::optional<MeshPath> findPath(MacAddress destination) const;
    void discard(Packet *packet);
    void deliver(Packet *packet, MacAddress source, MacAddress destination, int tid);
    void sendResolved(Packet *packet, MacAddress destination, const std::optional<MeshPath>& path);
    void expirePending();
    void scheduleExpiry();
    void clearPending();
    bool hasPending(MacAddress destination) const;
    Pending removePending(std::list<Pending>::iterator it);

  public:
    virtual ~MeshForwarding();
    virtual void processSourceMsdu(Packet *packet) override;
    virtual void processDataFrame(Packet *packet, const Ptr<const Ieee80211DataHeader>& header) override;
    virtual void pathAvailable(MacAddress destination) override;
    virtual void discoveryFailed(MacAddress destination) override;
    virtual bool hasPendingSource(MacAddress destination) override;
    int getNumPendingPackets() const { return pending.size(); }
    b getPendingBytes() const { return pendingBytes; }
};

} // namespace ieee80211
} // namespace inet
#endif
