//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211MIB_H
#define __INET_IEEE80211MIB_H

#include <memory>

#include "inet/common/SimpleModule.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211HtCapabilities.h"

namespace inet {

namespace physicallayer {
class Ieee80211ModeSet;
class IIeee80211Band;
}

namespace ieee80211 {

class INET_API Ieee80211Mib : public SimpleModule
{
  public:
    static simsignal_t bssStateChangedSignal;

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
        std::shared_ptr<const Ieee80211NegotiatedHtCapabilities> negotiatedCapabilities;
    };

  public:
    MacAddress address;
    Mode mode = static_cast<Mode>(-1);
    bool qos = false;

  private:
    BssData bssData;
    BssStationData bssStationData;
    BssAccessPointData bssAccessPointData;

    // This is a deliberately model-backed subset, not a full Annex C HT MIB implementation.
    bool localHtCapabilitiesValid = false;
    Ieee80211HtCapabilities localHtCapabilities;

  private:
    Ieee80211HtOperation htOperation;
    bool localCapabilitiesPrepared = false;
    bool primaryChannelAvailable = false;
    bool bssActive = false;
    bool htOperationPresent = false;
    const physicallayer::IIeee80211Band *operationBand = nullptr;
    bool stateChangePending = false;
    bool publishingStateChange = false;

    void checkStateMutation() const;
    std::map<MacAddress, short> associationIdReservations;
    std::map<MacAddress, PeerHtState> peerHtStates;

  protected:
    virtual void initialize(int stage) override;

  public:
    const BssData& getBssData() const { return bssData; }
    const BssStationData& getBssStationData() const { return bssStationData; }
    const BssAccessPointData& getBssAccessPointData() const { return bssAccessPointData; }
    const Ieee80211HtCapabilities& getLocalHtCapabilities() const { return localHtCapabilities; }
    void configureBssRole(BssStationType stationType, const std::string& ssid = "");
    void setAssociated(bool associated);
    BssMemberStatus getPeerAssociationStatus(const MacAddress& address) const;
    void setPeerAssociationStatus(const MacAddress& address, BssMemberStatus status);
    void removePeerAssociation(const MacAddress& address);
    static const char *getModeStr(Ieee80211Mib::Mode mode);
    static const char *getStationTypeStr(Ieee80211Mib::BssStationType stationType);
    std::string getSsidStr() const;
    short reserveAssociationId(const MacAddress& address);
    short commitAssociationId(const MacAddress& address);
    void cancelAssociationIdReservation(const MacAddress& address);
    short allocateAssociationId(const MacAddress& address);
    void releaseAssociationId(const MacAddress& address);
    void clearAssociationIds();
    // Initialization/preparation only. A changed profile requires inactive BSS and no peers.
    void installLocalHtCapabilities(const Ieee80211HtCapabilities& capabilities, bool htSupported);
    // Explicit coordinated reconfiguration; ordinary preparation remains guarded.
    void reconfigureLocalHtCapabilities(const Ieee80211HtCapabilities& capabilities, bool htSupported);
    bool hasPreparedLocalCapabilities() const { return localCapabilitiesPrepared; }
    bool isLocalHtCapable() const { return localHtCapabilitiesValid; }
    bool hasActiveBss() const { return bssActive; }
    bool hasHtOperation() const { return bssActive && htOperationPresent; }
    const physicallayer::IIeee80211Band *getOperationBand() const { return operationBand; }
    void commitBss(const std::string& ssid, const MacAddress& bssid, const physicallayer::IIeee80211Band *band,
            int channel, const Ieee80211HtOperation *operation);
    void clearBss();
    // Management publishes only after its required transaction/timer bookkeeping.
    // Synchronous observers may query state; nested mutation is rejected.
    void publishStateChange();
    bool hasPrimaryChannel() const { return primaryChannelAvailable; }
    int requirePrimaryChannel() const;
    const Ieee80211HtOperation& getHtOperation() const;
    bool relationshipAllowsHt(const MacAddress& address) const;
    const PeerHtState *findPeerCapabilities(const MacAddress& address) const;
    const PeerHtState *findPeerHtState(const MacAddress& address) const;
    void setPeerHtCapabilities(const MacAddress& address, const Ieee80211HtCapabilities& capabilities);
    void removePeerHtCapabilities(const MacAddress& address);
    void clearPeerHtCapabilities();
};

} // namespace ieee80211

} // namespace inet

#endif
