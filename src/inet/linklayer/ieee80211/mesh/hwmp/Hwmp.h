// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_HWMP_H
#define INET_HWMP_H

#include <sstream>
#include <tuple>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshPathSelection.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshForwarding.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshLinkMetric.h"
#include "inet/linklayer/ieee80211/mgmt/IMeshManagement.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"

namespace inet::ieee80211 {

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
    static simsignal_t rootUpdatedSignal;

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
        std::optional<simtime_t> sourceDue;
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
    enum RootMode { NO_ROOT = 0, PREQ_NO_PREP = 2, PREQ_WITH_PREP = 3, RANN = 4 };
    RootMode rootMode = NO_ROOT;
    simtime_t rootInterval;
    simtime_t rannInterval; // one station-wide mutable MIB scalar
    simtime_t confirmationInterval;
    simtime_t activePathToRootTimeout;
  public:
    struct Root {
        std::optional<HwmpPreq> invitation;
        bool responseSent = false;
        bool responseRequired = false;
        std::optional<HwmpRann> announcement;
        MacAddress transmitter;
        uint32_t metric = UINT32_MAX;
        std::optional<simtime_t> confirmationRequested;
        std::optional<simtime_t> propagationRequested;
        std::optional<simtime_t> lastPropagation;
        bool operator==(const Root& other) const {
            auto preqKey = [](const HwmpPreq& p) { return std::make_tuple(p.getFlags(),p.getHopCount(),p.getTtl(),p.getDiscoveryId(),p.getOriginatorSequenceNumber(),p.getLifetime(),p.getMetric()); };
            auto rannKey = [](const HwmpRann& r) { return std::make_tuple(r.flags,r.hopCount,r.ttl,r.root,r.sequenceNumber,r.interval,r.metric); };
            bool sameInvitation = invitation.has_value() == other.invitation.has_value() && (!invitation || preqKey(*invitation) == preqKey(*other.invitation));
            bool sameAnnouncement = announcement.has_value() == other.announcement.has_value() && (!announcement || rannKey(*announcement) == rannKey(*other.announcement));
            return sameInvitation && sameAnnouncement && responseSent == other.responseSent && responseRequired == other.responseRequired
                && transmitter == other.transmitter && metric == other.metric && confirmationRequested == other.confirmationRequested
                && propagationRequested == other.propagationRequested && lastPropagation == other.lastPropagation;
        }
        friend std::ostream& operator<<(std::ostream& out, const Root& root) {
            out << "invitation=";
            if (root.invitation) out << root.invitation->getOriginatorSequenceNumber() << ":" << root.invitation->getDiscoveryId() << " lifetimeTU=" << root.invitation->getLifetime();
            else out << "none";
            out << " responseSent=" << root.responseSent << " responseRequired=" << root.responseRequired << " RANN=";
            if (root.announcement) out << root.announcement->sequenceNumber << " hops=" << int(root.announcement->hopCount) << " ttl=" << int(root.announcement->ttl)
                << " wireIntervalTU=" << root.announcement->interval << " metric=" << root.metric << " transmitter=" << root.transmitter;
            else out << "none";
            if (root.confirmationRequested) out << " confirmationRequested=" << *root.confirmationRequested;
            if (root.propagationRequested) out << " propagationRequested=" << *root.propagationRequested;
            if (root.lastPropagation) out << " lastPropagation=" << *root.lastPropagation;
            return out;
        }
    };
    struct RootGlobals {
        simtime_t interval;
        std::optional<simtime_t> rootPreq, ownRann, rannAction, confirmationAction;
        bool localRannPending = false;
        bool operator==(const RootGlobals& other) const {
            return std::tie(interval,rootPreq,ownRann,rannAction,confirmationAction,localRannPending)
                == std::tie(other.interval,other.rootPreq,other.ownRann,other.rannAction,other.confirmationAction,other.localRannPending);
        }
    };
    class RootChange : public cObject {
      public:
        const MacAddress address;
        const std::optional<Root> root;
        const RootGlobals globals;
        const std::optional<simtime_t> lastTargetPreq;
        const std::optional<simtime_t> confirmationNotBefore;
        RootChange(MacAddress address, const std::optional<Root>& root, const RootGlobals& globals,
                std::optional<simtime_t> lastTargetPreq, std::optional<simtime_t> confirmationNotBefore) :
            address(address), root(root), globals(globals), lastTargetPreq(lastTargetPreq), confirmationNotBefore(confirmationNotBefore) {}
    };

  protected:
    struct RootObservation {
        std::map<MacAddress,Root> roots;
        std::map<MacAddress,std::optional<simtime_t>> targetPreqs;
        std::map<MacAddress,bool> usable;
        RootGlobals globals;
    };
    std::map<MacAddress,Root> roots;
    cMessage *rootOriginTimer = nullptr;
    cMessage *rootWorkTimer = nullptr;
    bool localRannPending = false;
    bool rootWorkDeferred = false; // synchronous control/batch transaction, never a deferred event
    std::optional<simtime_t> lastRootPreq;
    std::optional<simtime_t> lastOwnRann;
    std::optional<simtime_t> lastRannAction;
    std::optional<simtime_t> lastConfirmationAction;

    void initialize(int stage) override;
    void handleMessageWhenUp(cMessage *message) override;
    void receiveSignal(cComponent *source, simsignal_t signal, bool value, cObject *details) override;
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
    void unsubscribeFailureNotifications();
    void invalidatePeer(MacAddress peer, bool closed);
    void receivePerr(const HwmpPerr& perr, MacAddress transmitter);
    void queueErrors(const std::vector<PendingError>& errors);
    void sendPendingErrors();
    void originateRoot();
    void processRootWork(std::optional<RootObservation> before = std::nullopt);
    RootObservation observeRootState() const;
    void publishRootChanges(const RootObservation& before);
    void scheduleRootOrigin();
    void schedulePreqWork(MacAddress destination);
    simtime_t confirmationDue(MacAddress destination) const;
    bool confirmationNeeded(MacAddress destination) const;
    void receiveRann(const HwmpRann& rann, MacAddress transmitter);
    void sendProactivePrep(MacAddress root);
    void clearRootWork();
    B rootBatchBudget() const;
    std::string describeRootWork() const;
    void sendRootConfirmations();
    void sendRanns();
    bool sourceAttemptEligible(MacAddress destination);
    void recordPreqHandoff(MacAddress destination, bool sourceAttempt, bool publish = true);
    void sendSourcePreq(MacAddress destination);
    bool isInitializeStage(int stage) const override { return stage == INITSTAGE_LINK_LAYER; }
    bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    void handleStartOperation(LifecycleOperation *operation) override;
    void handleStopOperation(LifecycleOperation *operation) override;
    void handleCrashOperation(LifecycleOperation *operation) override;
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
    ~Hwmp() override;
    static bool isNewer(uint32_t a, uint32_t b) { auto delta = uint32_t(a-b); return delta != 0 && delta < 0x80000000U; }
    static uint32_t addMetric(uint32_t a, uint32_t b) { return UINT32_MAX-a < b ? UINT32_MAX : a+b; }
    std::optional<MeshPath> findPath(MacAddress destination) const override;
    simtime_t getPrecursorExpiry(MacAddress destination, MacAddress precursor) const override;
    void refreshPath(MacAddress destination) override;
    void refreshPrecursor(MacAddress destination, MacAddress precursor) override;
    void startDiscovery(MacAddress destination) override;
    void cancelDiscovery(MacAddress destination) override;
    void processHwmp(const Ptr<const Ieee80211HwmpFrame>& body, MacAddress transmitter, MacAddress receiver) override;
    void reportUnknownTransit(MacAddress transmitter, MacAddress destination) override;
    void sourcePathNeeded(MacAddress destination) override;
    // Read-only stored-state snapshots for model inspection and diagnostic tools.
    // Route usability still belongs to findPath(), including exclusive expiry.
    std::optional<PathInfo> getPathInfo(MacAddress destination) const;
    bool isDiscoveryActive(MacAddress destination) const;
    simtime_t getDiscoveryDeadline(MacAddress destination) const;
    int getDiscoveryAttempts(MacAddress destination) const;
    uint32_t getSequenceNumber() const { return ownSequence; }
    size_t getSeenIdentityCount() const;
};

} // namespace inet::ieee80211
#endif
