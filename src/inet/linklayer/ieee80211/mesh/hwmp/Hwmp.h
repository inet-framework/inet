// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_HWMP_H
#define INET_HWMP_H

#include <sstream>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshPathSelection.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshForwarding.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshLinkMetric.h"
#include "inet/linklayer/ieee80211/mgmt/IMeshManagement.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"

namespace inet {
namespace ieee80211 {

class Hcf;

class INET_API Hwmp : public OperationalBase, public cListener, public IMeshPathSelection
{
  public:
    struct PathInfo {
        MacAddress nextHop;
        std::optional<uint32_t> sequence;
        uint32_t metric = UINT32_MAX;
        unsigned int hops = 0;
        simtime_t expiry;
        bool validated = false;
        std::map<MacAddress,simtime_t> precursors;
        bool operator==(const PathInfo& other) const {
            return nextHop == other.nextHop && sequence == other.sequence && metric == other.metric && hops == other.hops
                && expiry == other.expiry && validated == other.validated && precursors == other.precursors;
        }
        bool operator!=(const PathInfo& other) const { return !(*this == other); }
        friend std::ostream& operator<<(std::ostream& out, const PathInfo& path) {
            out << "nextHop=" << path.nextHop << " sequence=";
            if (path.sequence) out << *path.sequence; else out << "unknown";
            out << " metric=" << path.metric << " hops=" << path.hops << " expiry=" << path.expiry << " validated=" << path.validated << " precursors={";
            for (const auto& entry : path.precursors) out << entry.first << ":" << entry.second << " ";
            return out << "}";
        }
    };
    class PathChange : public cObject {
      public:
        const MacAddress destination;
        const std::optional<PathInfo> path;
        const std::map<MacAddress,simtime_t> selfAdmission;
        PathChange(MacAddress destination, const std::optional<PathInfo>& path, const std::map<MacAddress,simtime_t>& selfAdmission) :
            destination(destination), path(path), selfAdmission(selfAdmission) {}
    };
    class DiscoveryChange : public cObject {
      public:
        const MacAddress destination;
        const int attempts;
        const simtime_t nextAttempt;
        const std::optional<simtime_t> lastPreq;
        DiscoveryChange(MacAddress destination, int attempts, simtime_t nextAttempt, std::optional<simtime_t> lastPreq) :
            destination(destination), attempts(attempts), nextAttempt(nextAttempt), lastPreq(lastPreq) {}
    };
    static simsignal_t pathUpdatedSignal;
    static simsignal_t discoveryChangedSignal;

  protected:
    ModuleRefByPar<Ieee80211Mib> mib;
    ModuleRefByPar<IMeshManagement> management;
    ModuleRefByPar<IMeshLinkMetric> linkMetric;
    ModuleRefByPar<IMeshForwarding> forwarding;
    ModuleRefByPar<Hcf> hcf;
    std::map<MacAddress,PathInfo> paths;
    std::map<MacAddress,simtime_t> selfPrecursors;
    std::map<std::pair<MacAddress,uint32_t>,simtime_t> seen;
    struct Discovery {
        cMessage *timer = nullptr;
        std::optional<int> attempts; // absent while idle; timer object and minimum-interval history remain
        std::optional<simtime_t> lastSent;
    };
    std::map<MacAddress,Discovery> discoveries;
    uint32_t ownSequence = 0;
    uint32_t nextDiscoveryId = 0;
    simtime_t preqMinInterval;
    simtime_t traversalTime;
    simtime_t activePathTimeout;
    int maxRetries = 0;
    int initialTtl = 0;
    int seenCapacity = 0;
    cMessage *pathExpiryTimer = nullptr;
    cMessage *perrTimer = nullptr;
    simtime_t perrMinInterval;
    std::optional<simtime_t> lastPerrHandoff;
    struct PendingError {
        MacAddress recipient;
        uint8_t ttl = 0;
        HwmpPerrDestination destination;
        simtime_t expiry;
    };
    std::vector<PendingError> pendingErrors;

    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *message) override;
    void receiveSignal(cComponent *source, simsignal_t signal, bool value, cObject *details) override;
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
    void unsubscribeFailureNotifications();
    void invalidatePeer(MacAddress peer, bool closed);
    void receivePerr(const HwmpPerr& perr, MacAddress transmitter);
    void queueErrors(const std::vector<PendingError>& errors);
    void sendPendingErrors();
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    void stopDiscoveries();
    void attemptDiscovery(MacAddress destination);
    void expireSeen();
    std::string describeDiscoveries() const;
    void publishPath(MacAddress destination);
    void publishPathChanges(const std::map<MacAddress,std::optional<PathInfo>>& before, const std::map<MacAddress,simtime_t>& priorSelf);
    void publishDiscovery(MacAddress destination);
    void schedulePathExpiry();
    bool stale(MacAddress destination, uint32_t sequence) const;
    bool updatePath(MacAddress destination, MacAddress transmitter, uint32_t sequence, uint32_t metric, unsigned int hops, simtime_t expiry);
    void addPrecursor(MacAddress destination, MacAddress precursor, simtime_t expiry);
    void notifyUsable(MacAddress destination);
    void receivePreq(const HwmpPreq& preq, MacAddress transmitter, MacAddress receiver);
    void receivePrep(const HwmpPrep& prep, MacAddress transmitter);
    void reply(const HwmpPreq& preq, const HwmpPreqTarget& target, bool local);
    void sendPreq(const HwmpPreq& preq, MacAddress receiver);
    void sendPrep(const HwmpPrep& prep, MacAddress receiver);

  public:
    virtual ~Hwmp();
    static bool isNewer(uint32_t a, uint32_t b) { auto delta = uint32_t(a-b); return delta != 0 && delta < 0x80000000U; }
    static uint32_t addMetric(uint32_t a, uint32_t b) { return UINT32_MAX-a < b ? UINT32_MAX : a+b; }
    virtual std::optional<MeshPath> findPath(MacAddress destination) const override;
    virtual simtime_t getPrecursorExpiry(MacAddress destination, MacAddress precursor) const override;
    virtual void refreshPath(MacAddress destination) override;
    virtual void refreshPrecursor(MacAddress destination, MacAddress precursor) override;
    virtual void startDiscovery(MacAddress destination) override;
    virtual void cancelDiscovery(MacAddress destination) override;
    virtual void processHwmp(const Ptr<const Ieee80211HwmpFrame>& body, MacAddress transmitter, MacAddress receiver) override;
    void reportUnknownTransit(MacAddress transmitter, MacAddress destination) override;
    // Read-only stored-state snapshots for model inspection and diagnostic tools.
    // Route usability still belongs to findPath(), including exclusive expiry.
    std::optional<PathInfo> getPathInfo(MacAddress destination) const;
    bool isDiscoveryActive(MacAddress destination) const;
    simtime_t getDiscoveryDeadline(MacAddress destination) const;
    int getDiscoveryAttempts(MacAddress destination) const;
    uint32_t getSequenceNumber() const { return ownSequence; }
    size_t getSeenIdentityCount() const;
};

} // namespace ieee80211
} // namespace inet
#endif
