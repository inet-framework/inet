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
    struct PeerWakeState {
        bool rendezvous = false;
        bool groupBuffered = false;
        bool individualBuffered = false;
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
    // Production activation requires P3's complete service-period engine.
    // A physically awake protocol fixture can supply explicit opportunities.
    virtual bool isActivationSupported() const { return false; }
    void openAwakeOpportunity(MacAddress peer, simtime_t end);
    MeshPeerInstance instance(MacAddress peer) const;
    Packet *makeModeIndication(MacAddress receiver, MeshPowerMode mode) const;
    bool hasBufferedGroup(const std::set<int64_t>& exclude) const;
    bool hasSleepingPeer() const;
    void clearPolicy();

  public:
    static simsignal_t meshAwakeRequirementChangedSignal;
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
};

} // namespace inet::ieee80211
#endif
