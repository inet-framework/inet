// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_HWMP_ROOT_SUPPORT_H
#define INET_TEST_HWMP_ROOT_SUPPORT_H
#include "mesh/HwmpErrorSupport.h"

class HwmpRootProbe : public Hwmp
{
  public:
    bool hasInvitation(MacAddress address) const { auto it = roots.find(address); return it != roots.end() && it->second.invitation.has_value(); }
    bool responseSent(MacAddress address) const { auto it = roots.find(address); return it != roots.end() && it->second.responseSent; }
    bool hasAnnouncement(MacAddress address) const { auto it = roots.find(address); return it != roots.end() && it->second.announcement.has_value(); }
    MacAddress rannSender(MacAddress address) const { return roots.at(address).transmitter; }
    uint32_t rannMetric(MacAddress address) const { return roots.at(address).metric; }
    uint32_t wireInterval(MacAddress address) const { return roots.at(address).announcement->interval; }
    bool confirmationPending(MacAddress address) const { auto it = roots.find(address); return it != roots.end() && it->second.confirmationRequested.has_value(); }
    bool propagationPending(MacAddress address) const { auto it = roots.find(address); return it != roots.end() && it->second.propagationRequested.has_value(); }
    uint32_t sharedIntervalTus() const { return rannInterval.inUnit(SIMTIME_US)/1024; }
    uint32_t confirmationIntervalTus() const { return confirmationInterval.inUnit(SIMTIME_US)/1024; }
    std::optional<simtime_t> lastRann() const { return lastRannAction; }
    std::optional<simtime_t> lastConfirmation() const { return lastConfirmationAction; }
    std::optional<simtime_t> lastTargetPreq(MacAddress address) const { auto it = discoveries.find(address); return it == discoveries.end() ? std::nullopt : it->second.lastSent; }
    uint32_t groupIdCounter() const { return nextDiscoveryId; }
    bool hasLocalRootTimer() const { return rootOriginTimer != nullptr; }
    bool localRootTimerScheduled() const { return rootOriginTimer && rootOriginTimer->isScheduled(); }
    bool rootWorkScheduled() const { return rootWorkTimer->isScheduled(); }
    simtime_t rootWorkDeadline() const { return rootWorkTimer->isScheduled() ? rootWorkTimer->getArrivalTime() : SIMTIME_ZERO; }
    uint32_t invitationSn(MacAddress address) const { return roots.at(address).invitation->getOriginatorSequenceNumber(); }
    std::optional<simtime_t> lastPropagation(MacAddress address) const { return roots.at(address).lastPropagation; }
    std::optional<Root> rootSnapshot(MacAddress address) const { auto it = roots.find(address); return it == roots.end() ? std::nullopt : std::optional<Root>(it->second); }
    RootGlobals globalsSnapshot() const { return {rannInterval,lastRootPreq,lastOwnRann,lastRannAction,lastConfirmationAction,localRannPending}; }
    std::optional<simtime_t> confirmationDeadline(MacAddress address) const { return confirmationPending(address) ? std::optional<simtime_t>(confirmationDue(address)) : std::nullopt; }
};
Define_Module(HwmpRootProbe);
inline HwmpRootProbe *rootProbe(int station) { return check_and_cast<HwmpRootProbe *>(hwmp(station)); }

class RootFactChecks
{
    struct Last {
        std::optional<Hwmp::Root> root;
        Hwmp::RootGlobals globals;
        std::optional<simtime_t> target, deadline;
        bool usable;
    };
    std::map<std::pair<cComponent *,MacAddress>,Last> previous;
  protected:
    int rootEvents = 0;
    void checkRootFact(cComponent *source, bool value, cObject *details) {
        auto owner = check_and_cast<HwmpRootProbe *>(source); auto change = check_and_cast<Hwmp::RootChange *>(details);
        ASSERT(change->root == owner->rootSnapshot(change->address) && change->globals == owner->globalsSnapshot());
        ASSERT(value == owner->findPath(change->address).has_value());
        ASSERT(change->lastTargetPreq == (change->root ? owner->lastTargetPreq(change->address) : std::nullopt));
        ASSERT(change->confirmationNotBefore == owner->confirmationDeadline(change->address));
        auto key = std::make_pair(source,change->address); auto prior = previous.find(key);
        if (prior != previous.end()) ASSERT(!(prior->second.root == change->root && prior->second.globals == change->globals
            && prior->second.target == change->lastTargetPreq && prior->second.deadline == change->confirmationNotBefore && prior->second.usable == value));
        previous[key] = {change->root,change->globals,change->lastTargetPreq,change->confirmationNotBefore,value}; rootEvents++;
    }
};

class RootTestManagement : public HwmpTestManagement
{
    bool holding = false;
    bool droppingRanns = false;
    std::optional<MacAddress> heldOriginator;
    std::vector<std::pair<Ptr<Ieee80211HwmpFrame>,MacAddress>> held;
  protected:
    void sendDown(Packet *packet) override {
        auto body = dynamicPtrCast<const Ieee80211HwmpFrame>(packet->peekAtFront<Chunk>());
        if (droppingRanns && body && body->getRannsArraySize()) { emit(registerSignal("hwmpTestHandoff"),packet); delete packet; }
        else if (holding && body && body->getPrepsArraySize() && (!heldOriginator || body->getPreps(0).originator == *heldOriginator)) {
            emit(registerSignal("hwmpTestHandoff"),packet);
            held.emplace_back(makeShared<Ieee80211HwmpFrame>(*body),packet->getTag<MacAddressReq>()->getDestAddress());
            delete packet;
        }
        else HwmpTestManagement::sendDown(packet);
    }
  public:
    void holdPreps(bool value) { Enter_Method("holdPreps"); holding = value; heldOriginator.reset(); }
    void holdPrepsFor(MacAddress originator) { Enter_Method("holdPrepsFor"); holding = true; heldOriginator = originator; }
    void dropRanns(bool value) { Enter_Method("dropRanns"); droppingRanns = value; }
    size_t heldCount() const { return held.size(); }
    void releasePreps() {
        Enter_Method("releasePreps"); holding = false; auto releasing = std::move(held); held.clear();
        for (const auto& entry : releasing) sendTestBody(entry.first,entry.second);
    }
};
Define_Module(RootTestManagement);
inline RootTestManagement *rootManagement(int station) { return check_and_cast<RootTestManagement *>(meshManagement(station)); }
inline Ptr<Ieee80211HwmpFrame> rootRannBody(MacAddress address, uint32_t sn, uint32_t interval, uint32_t metric = 0, uint8_t ttl = 8) {
    auto body = makeShared<Ieee80211HwmpFrame>(); HwmpRann rann; rann.root = address; rann.sequenceNumber = sn; rann.interval = interval; rann.metric = metric; rann.ttl = ttl;
    body->setRannsArraySize(1); body->setRanns(0,rann); body->setChunkLength(B(23)); return body;
}
#endif
