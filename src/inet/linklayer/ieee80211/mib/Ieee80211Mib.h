//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211MIB_H
#define __INET_IEEE80211MIB_H

#include "inet/common/SimpleModule.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211HtCapabilities.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211RateSet.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211RateSetChangeDetails.h"

namespace inet {

namespace physicallayer {
class Ieee80211ModeSet;
class IIeee80211Band;
}

namespace ieee80211 {

class INET_API Ieee80211Mib : public SimpleModule
{
  public:
    enum Mode {
        INFRASTRUCTURE,
        INDEPENDENT,
        MESH
    };

    enum BssStationType {
        ACCESS_POINT,
        STATION
    };

    enum BssMemberStatus {
        NOT_AUTHENTICATED,
        AUTHENTICATED,
        ASSOCIATED
    };

    class INET_API BssData {
      public:
        std::string ssid;
        MacAddress bssid;
    };

    class INET_API BssStationData {
      public:
        BssStationType stationType = static_cast<BssStationType>(-1);
        bool isAssociated = false;
    };

    class INET_API BssAccessPointData {
      public:
        std::map<MacAddress, BssMemberStatus> stations;
        std::map<MacAddress, short> associationIds;
    };

    class INET_API PeerHtState {
      public:
        bool valid = false;
        Ieee80211HtCapabilities advertisedCapabilities;
        Ieee80211NegotiatedHtCapabilities negotiatedCapabilities;
        uint64_t generation = 0;
    };

  public:
    MacAddress address;
    Mode mode = static_cast<Mode>(-1);
    bool qos = false;

    BssData bssData;
    BssStationData bssStationData;
    BssAccessPointData bssAccessPointData;

    // This is a deliberately model-backed subset, not a full Annex C HT MIB implementation.
    bool localHtCapabilitiesValid = false;
    Ieee80211HtCapabilities localHtCapabilities;

  private:
    Ieee80211HtOperation htOperation;
    int configuredSecondaryChannelOffset = 0;
    bool primaryChannelAvailable = false;
    std::map<MacAddress, short> associationIdReservations;
    std::map<MacAddress, PeerHtState> peerHtStates;
    Ieee80211RateSetState localRateSet;
    Ieee80211RateSetState bssRateSet;
    std::map<MacAddress, Ieee80211RateSetState> peerRateSets;
    uint64_t rateSetRevision = 0;

    void incrementRateSetRevision();

  protected:
    virtual void initialize(int stage) override;

  public:
    static simsignal_t bssRateSetChangedSignal;
    static simsignal_t peerRateSetChangedSignal;

    static const char *getModeStr(Ieee80211Mib::Mode mode);
    static const char *getStationTypeStr(Ieee80211Mib::BssStationType stationType);
    std::string getSsidStr() const;
    short reserveAssociationId(const MacAddress& address);
    short commitAssociationId(const MacAddress& address);
    void cancelAssociationIdReservation(const MacAddress& address);
    short allocateAssociationId(const MacAddress& address);
    void releaseAssociationId(const MacAddress& address);
    void clearAssociationIds();
    void updateLocalHtCapabilities(const physicallayer::Ieee80211ModeSet *modeSet,
            const std::set<Hz>& operationalChannelWidths, int operationalHtSpatialStreamLimit);
    bool isHtOperationSupported() const { return localHtCapabilitiesValid; }
    bool hasPrimaryChannel() const { return primaryChannelAvailable; }
    int requirePrimaryChannel() const;
    void setPrimaryChannel(int primaryChannel);
    void setPrimaryChannel(int primaryChannel, const physicallayer::IIeee80211Band *band);
    const Ieee80211HtOperation& getHtOperation() const;
    const PeerHtState *findPeerHtState(const MacAddress& address) const;
    void setPeerHtCapabilities(const MacAddress& address, const Ieee80211HtCapabilities& capabilities, const Ieee80211HtOperation& operation);
    void removePeerHtCapabilities(const MacAddress& address);
    void clearPeerHtCapabilities();

    // Views remain valid until the corresponding update or clear operation.
    const Ieee80211RateSetState& getLocalRateSet() const { return localRateSet; }
    const Ieee80211RateSetState& getBssRateSet() const { return bssRateSet; }
    uint64_t getRateSetRevision() const { return rateSetRevision; }
    const Ieee80211RateSetState *findPeerRateSet(const MacAddress& address) const;
    void setLocalRateSet(const Ieee80211RateSetState& rateSet);
    void setBssRateSet(const Ieee80211RateSetState& rateSet);
    void installBssAndPeerRateSets(const Ieee80211RateSetState& bssRateSet,
            const MacAddress& peerAddress, const Ieee80211RateSetState& peerRateSet);
    void clearBssRateSet();
    void removePeerRateSet(const MacAddress& address);
    void clearPeerRateSets();
};

} // namespace ieee80211

} // namespace inet

#endif
