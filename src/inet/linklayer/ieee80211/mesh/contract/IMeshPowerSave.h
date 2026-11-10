// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_IMESHPOWERSAVE_H
#define INET_IMESHPOWERSAVE_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mesh/MeshPowerMode.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MeshFrame_m.h"

namespace inet::ieee80211 {

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
};

} // namespace inet::ieee80211
#endif
