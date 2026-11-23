// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_IMESHPOWERSAVE_H
#define INET_IMESHPOWERSAVE_H

#include <optional>
#include <map>
#include <set>
#include <vector>
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mesh/MeshPowerMode.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MeshFrame_m.h"

namespace inet::ieee80211 {

enum class MeshPeriodDirection { OWNER = 0, RECIPIENT = 1 };
enum class MeshPeriodOutcome { CONFIRMED = 0, FAILED = 1, WITHDRAWN = 2 };
enum class MeshPowerCause {
    RESPONSE = 0, RESPONSE_EXPIRED = 1, RESPONSE_REJECTED = 2, BAR_PENDING = 3,
    BAR_RESPONSE = 4, BAR_EXHAUSTED = 5, EXPIRED = 6, WITHDRAWN = 7,
    AGREEMENT_DISPOSAL = 8, ORDINARY_LIMIT = 9
};

struct INET_API MeshPowerTxMember
{
    int64_t id = -1;
    MacAddress peer;
    int type = ST_DATA;
    Tid tid = 0;
    bool rspi = false;
    bool eosp = false;
    int ordinaryLimit = 0;
};

struct INET_API MeshPowerResolution
{
    MacAddress peer;
    Tid tid = 0;
    std::set<int64_t> originals;
    MeshPowerCause cause = MeshPowerCause::RESPONSE;
    std::map<int64_t, MeshPowerCause> retired;
    std::optional<bool> barPending; // Unset: a cause-only batch does not resolve recovery.
};

struct INET_API MeshPowerBaWindow
{
    SequenceNumberCyclic start;
    int size = 0;
};

/** Copied view of current state, never a historical trigger identity. */
struct INET_API MeshServicePeriodView
{
    MacAddress peer;
    MeshPeriodDirection direction;
    bool ending = false;
    int cap = 0;
    int spent = 0;
    std::set<int64_t> cohort;
};

/** Mesh power policy. MAC queues, timers and packet ownership stay with HCF. */
class INET_API IMeshPowerSave
{
  public:
    virtual ~IMeshPowerSave() = default;
    virtual bool isEnabled() const = 0;
    virtual void requestMode(MacAddress peer, MeshPowerMode mode) = 0;
    virtual void requestActiveForAllPeers() = 0;
    virtual MeshPowerMode getRequestedMode(MacAddress peer) const = 0;
    virtual bool isTransmissionEligible(const Packet *packet, simtime_t plannedStart) const = 0;
    virtual void prepareTransmission(Packet *packet, const std::set<int64_t>& deliveryUnit) = 0;
    virtual void originalAcknowledged(const Packet *packet) = 0;
    virtual void originalRetired(const Packet *packet) = 0;
    virtual void groupTransmissionComplete(const Packet *packet) = 0;
    virtual void receivePowerFields(const Packet *packet, const Ptr<const Ieee80211MeshDataHeader>& header) = 0;
    virtual void peerRemoved(MacAddress peer) = 0;
    virtual MeshTim getTim(uint8_t dtimCount, uint8_t dtimPeriod, const std::set<int64_t>& deliveryUnit) const = 0;
    virtual bool needsAwakeWindow(const MeshTim& tim) const = 0;
    virtual void dtimTransmissionComplete() = 0;
    virtual void reviewAwakeState() = 0;
    virtual bool requiresAwake() const = 0;
    virtual void radioObservationChanged() = 0;
    virtual void managementTimingChanged() = 0;
    virtual void timingTransmissionComplete(simtime_t advertisedWindow) = 0;
    virtual void timingTransmissionRetired() = 0;
    virtual void beaconReceived(MacAddress peer, bool groupBuffered, bool individualBuffered, simtime_t window) = 0;
    virtual void groupFrameReceived(MacAddress peer, bool moreData) = 0;
    virtual void ppduTransmissionComplete(const Packet *packet, simtime_t start) = 0;
    virtual void reviewServicePeriods() = 0;
    virtual void recipientMemberReceived(const Packet *packet, const Ptr<const Ieee80211MeshDataHeader>& header, bool duplicate) = 0;
    virtual bool canTransmitSet(const std::vector<MeshPowerTxMember>& members, simtime_t start) const = 0;
    virtual bool authorizeTransmissionStart(const std::vector<MeshPowerTxMember>& members, simtime_t start) = 0;
    virtual void transmissionResolved(const MeshPowerResolution& result) = 0;
    virtual void prepareRecipientResponse(Packet *response, const Packet *eliciting,
            const Ptr<const Ieee80211MacHeader>& header, const std::optional<MeshPowerBaWindow>& window) = 0;
    virtual void recipientResponseComplete(const Packet *response) = 0;
    virtual void recipientAgreementRetired(MacAddress peer, Tid tid) = 0;
};

} // namespace inet::ieee80211
#endif
