// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_MESHPOWERSAVE_H
#define INET_MESHPOWERSAVE_H

#include <array>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshPowerSave.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshPowerBuffer.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshDataMac.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshRadioControl.h"
#include "inet/linklayer/ieee80211/mgmt/IMeshManagement.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"

namespace inet::ieee80211 {

/** IEEE 802.11-2024, 14.15: peer modes and awake obligations. */
class INET_API MeshPowerSave : public OperationalBase, public IMeshPowerSave
{
  protected:
    ModuleRefByPar<Ieee80211Mib> mib;
    ModuleRefByPar<IMeshManagement> management;
    ModuleRefByPar<IMeshPowerBuffer> buffers;
    ModuleRefByPar<IMeshDataMac> mac;
    ModuleRefByPar<IMeshRadioControl> radioControl;
    bool enabled = false;
    struct Transition {
        MeshPowerMode mode;
        MeshPeerInstance instance;
        int64_t originalId;
        friend std::ostream& operator<<(std::ostream& out, const Transition& transition) {
            return out << "requested=" << int(transition.mode) << ", links=" << transition.instance.first
                << "/" << transition.instance.second << ", original=" << transition.originalId;
        }
    };
    std::map<MacAddress, Transition> transitions;
    std::map<MacAddress, simtime_t> awakeUntil;
    bool deliveringGroups = false;
    cMessage *boundaryTimer = nullptr;
    simtime_t wakeMargin;
    simtime_t ownAwEnd;
    simtime_t postAwEnd;
    simtime_t advertisedAw;
    std::optional<simtime_t> groupIdleSince;
    uint64_t maintenanceOrdinal = 0;
    bool ownRendezvous = false;
    bool awakeRequired = true;
    int missingAckRetryLimit = 7;
    struct Period {
        MeshPeerInstance instance;
        int64_t openingId = -1;
        simtime_t openingTime = -1;
        bool ending = false;
        int cap = 0;
        int spent = 0;
        std::set<int64_t> started;
        std::set<int64_t> cohort;
        std::map<int64_t, std::optional<MeshPowerCause>> retired;
        int64_t endingNull = -1;
        bool replacementCreated = false;
        bool barPending = false;
        int64_t pendingReceipt = -1;
        std::set<std::pair<Tid, SequenceNumber>> pendingEosp;
        friend std::ostream& operator<<(std::ostream& out, const Period& value) {
            return out << "ending=" << value.ending << " cap=" << value.cap << " spent=" << value.spent
                << " cohort=" << value.cohort.size() << " null=" << value.endingNull;
        }
    };
    struct Trigger {
        MeshPeerInstance instance;
        int64_t queuedId = -1;
        std::set<int64_t> attempted;
        int64_t openingId = -1;
        simtime_t start = -1;
        bool rspi = false;
        bool eosp = false;
        bool failed = false;
    };
    std::map<MacAddress, std::array<std::optional<Period>, 2>> periods;
    std::map<MacAddress, Trigger> triggers;
    struct PeerWakeState {
        bool rendezvous = false;
        bool groupBuffered = false;
        bool individualBuffered = false;
        bool retrievalRequested = false;
        friend std::ostream& operator<<(std::ostream& out, const PeerWakeState& state) {
            return out << "rendezvous=" << state.rendezvous << ", group=" << state.groupBuffered
                << ", individual=" << state.individualBuffered;
        }
    };
    std::map<MacAddress, PeerWakeState> peerWake;

    void initialize(int stage) override;
    void preDelete(cComponent *root) override;
    void handleMessageWhenUp(cMessage *message) override;
    bool isInitializeStage(int stage) const override { return stage == INITSTAGE_LINK_LAYER; }
    bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    void handleStartOperation(LifecycleOperation *operation) override;
    void handleStopOperation(LifecycleOperation *operation) override;
    void handleCrashOperation(LifecycleOperation *operation) override;
    virtual bool isActivationSupported() const { return true; }
    void openAwakeOpportunity(MacAddress peer, simtime_t end);
    MeshPeerInstance instance(MacAddress peer) const;
    Packet *makeModeIndication(MacAddress receiver, MeshPowerMode mode) const;
    bool hasBufferedGroup(const std::set<int64_t>& exclude) const;
    bool hasSleepingPeer() const;
    void clearPolicy();
    Period *findPeriod(MacAddress peer, MeshPeriodDirection direction);
    const Period *findPeriod(MacAddress peer, MeshPeriodDirection direction) const;
    void openPeriod(MacAddress peer, MeshPeriodDirection direction, int64_t id, simtime_t time);
    std::optional<Period> detachRecipient(MacAddress peer);
    void retirePeriod(MacAddress peer, MeshPeriodDirection direction, MeshPeriodOutcome outcome);
    void applyTrigger(MacAddress peer, bool rspi, bool eosp, int64_t id, simtime_t time);
    void queueServiceControl(MacAddress peer, bool rspi, bool eosp, bool ending);
    bool peerAwake(MacAddress peer, simtime_t start) const;
    bool canInitiateTo(MacAddress peer, simtime_t start) const;

  public:
    static simsignal_t meshAwakeRequirementChangedSignal;
    static simsignal_t meshServicePeriodOutcomeSignal;
    class PeriodOutcome : public cObject {
      public:
        MacAddress peer;
        MeshPeriodDirection direction;
        MeshPeriodOutcome outcome;
        int64_t openingId;
        simtime_t openingTime;
        PeriodOutcome(MacAddress peer, MeshPeriodDirection direction, MeshPeriodOutcome outcome,
                int64_t openingId, simtime_t openingTime) :
            peer(peer), direction(direction), outcome(outcome), openingId(openingId), openingTime(openingTime) {}
    };
    ~MeshPowerSave() override;
    bool isEnabled() const override { return enabled && operationalState == OPERATING; }
    void requestMode(MacAddress peer, MeshPowerMode mode) override;
    void requestActiveForAllPeers() override;
    MeshPowerMode getRequestedMode(MacAddress peer) const override;
    bool isTransmissionEligible(const Packet *packet, simtime_t plannedStart) const override;
    void prepareTransmission(Packet *packet, const std::set<int64_t>& deliveryUnit) override;
    void originalAcknowledged(const Packet *packet) override;
    void originalRetired(const Packet *packet) override;
    void groupTransmissionComplete(const Packet *packet) override;
    void receivePowerFields(const Packet *packet, const Ptr<const Ieee80211MeshDataHeader>& header) override;
    void peerRemoved(MacAddress peer) override;
    MeshTim getTim(uint8_t dtimCount, uint8_t dtimPeriod, const std::set<int64_t>& deliveryUnit) const override;
    bool needsAwakeWindow(const MeshTim& tim) const override;
    void dtimTransmissionComplete() override;
    void reviewAwakeState() override;
    bool requiresAwake() const override { return !isEnabled() || awakeRequired; }
    void radioObservationChanged() override;
    void managementTimingChanged() override;
    void timingTransmissionComplete(simtime_t window) override;
    void timingTransmissionRetired() override;
    void beaconReceived(MacAddress peer, bool groupBuffered, bool individualBuffered, simtime_t window) override;
    void groupFrameReceived(MacAddress peer, bool moreData) override;
    void ppduTransmissionComplete(const Packet *packet, simtime_t start) override;
    void reviewServicePeriods() override;
    void recipientMemberReceived(const Packet *packet, const Ptr<const Ieee80211MeshDataHeader>& header, bool duplicate) override;
    bool canTransmitSet(const std::vector<MeshPowerTxMember>& members, simtime_t start) const override;
    bool authorizeTransmissionStart(const std::vector<MeshPowerTxMember>& members, simtime_t start) override;
    void transmissionResolved(const MeshPowerResolution& result) override;
    void prepareRecipientResponse(Packet *response, const Packet *eliciting,
            const Ptr<const Ieee80211MacHeader>& header, const std::optional<MeshPowerBaWindow>& window) override;
    void recipientResponseComplete(const Packet *response) override;
    void recipientAgreementRetired(MacAddress peer, Tid tid) override;
    std::vector<MeshServicePeriodView> getServicePeriods() const;
};

} // namespace inet::ieee80211
#endif
