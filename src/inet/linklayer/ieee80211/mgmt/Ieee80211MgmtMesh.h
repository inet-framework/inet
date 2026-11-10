// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_IEEE80211MGMTMESH_H
#define __INET_IEEE80211MGMTMESH_H

#include <optional>

#include "inet/linklayer/ieee80211/mgmt/IMeshManagement.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshLinkMetric.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshLinkTiming.h"
#include "inet/linklayer/ieee80211/mesh/contract/IMeshPathSelection.h"

namespace inet {
namespace ieee80211 {

class INET_API Ieee80211MgmtMesh : public Ieee80211MgmtBase, public IMeshManagement
{
  public:
    enum PeerState { IDLE = 0, OPN_SNT = 1, CNF_RCVD = 2, OPN_RCVD = 3, ESTAB = 4, HOLDING = 5 };
    static simsignal_t meshPeeringStateChangedSignal;
    static simsignal_t meshManagementDiscardedSignal;
    static simsignal_t meshBeaconTransmissionEndSignal;

  protected:
    enum Event { ACTIVE_OPEN, OPEN_ACCEPT, CONFIRM_ACCEPT, CLOSE_ACCEPT, OPEN_REJECT,
        CONFIRM_REJECT, REQUEST_REJECT, CANCEL, RETRY_EXPIRED, CONFIRM_EXPIRED, HOLDING_EXPIRED };
    struct Peer {
        PeerState state = IDLE;
        uint16_t localLinkId = 0;
        std::optional<uint16_t> peerLinkId;
        uint16_t localAid = 0;
        uint16_t peerAid = 0;
        uint16_t closeReason = 0;
        int retries = 0;
        cMessage *timer = nullptr;
        Ieee80211RateSetState rates;
        Ieee80211PeerPhyCapabilities phy;
    };
    struct Neighbor {
        bool offsetValid = false;
        int64_t offset = 0; // microseconds, neighbor TSF minus local TSF
    };
    std::map<MacAddress, Peer> peers;
    ModuleRefByPar<IMeshLinkMetric> linkMetric;
    ModuleRefByPar<IMeshLinkTiming> linkTiming;
    ModuleRefByPar<IMeshPathSelection> pathSelection;
    std::map<MacAddress, Neighbor> neighbors;
    cMessage *beaconTimer = nullptr;
    simtime_t beaconInterval;
    simtime_t retryTimeout;
    simtime_t confirmTimeout;
    simtime_t holdingTimeout;
    simtime_t tsfOrigin;
    simtime_t totalSlowing;
    simtime_t lastSlowingStart;
    simtime_t lastSlowingDuration;
    simtime_t nextTbttTsf;
    simtime_t maxClockDrift; // retained residual between beacons, including fractional-us budget
    int maxRetries = 0;
    int maxPeerings = 0;
    int dtimPeriod = 1;
    uint16_t awakeWindow = 0; // TUs
    uint16_t nextLinkId = 1;
    uint64_t beaconNumber = 0;
    int64_t pendingBeaconTreeId = -1;
    bool autoPeer = true;

    virtual void initialize(int stage) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
    virtual void handleMessageWhenUp(cMessage *message) override;
    void handleMessageWhenDown(cMessage *message) override;
    virtual void handleTimer(cMessage *message) override;
    virtual void handleCommand(int kind, cObject *control) override;
    virtual void start() override;
    virtual void stop() override;
    virtual void dropManagementFrame(Packet *packet) override;
    void processManagement(Packet *packet);
    void processBeacon(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header, bool response);
    void processPeering(Packet *packet, const Ptr<const Ieee80211MeshActionFrame>& header);
    void processLinkMetric(Packet *packet, const Ptr<const Ieee80211MeshActionFrame>& header);
    void processHwmpFrame(Packet *packet, const Ptr<const Ieee80211MeshActionFrame>& header);
    void processProbeRequest(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header);
    void updateNeighborTiming(Packet *packet, const Ptr<const Ieee80211BeaconFrame>& body, MacAddress peer);
    bool matchesProfile(const Ieee80211MeshFrame& body) const;
    bool compatibleRates(const Ieee80211RateSetState& rates) const;
    bool decodePeerPhy(const Ieee80211MeshFrame& body, Ieee80211PeerPhyCapabilities& phy) const;
    Ieee80211RateSetState readPeeringRates(const Ieee80211MeshPeeringOpenFrame& body) const;
    Peer& requirePeer(MacAddress address);
    void createInstance(Peer& peer);
    void setState(MacAddress address, Peer& peer, PeerState state);
    void retirePeer(MacAddress address);
    void event(MacAddress address, Event event, uint16_t reason = 0);
    void arm(Peer& peer, simtime_t timeout);
    uint16_t allocateAid() const;
    void sendPeering(MacAddress address, uint8_t action, uint16_t reason = 0);
    void sendBeacon(bool response = false, MacAddress destination = MacAddress::BROADCAST_ADDRESS);
    void sendBody(const Ptr<Ieee80211MeshFrame>& body, Ieee80211FrameType subtype, MacAddress destination);
    void setMeshElements(const Ptr<Ieee80211MeshFrame>& body) const;
    void scheduleNextBeacon();
    void applyClockSlowing();
    simtime_t tsfAt(simtime_t time) const;
    std::string describePeers() const;

    // Infrastructure-only subtypes are unsupported in this mesh profile.
    virtual void handleAuthenticationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>&) override;
    virtual void handleDeauthenticationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>&) override;
    virtual void handleAssociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>&) override;
    virtual void handleAssociationResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>&) override;
    virtual void handleReassociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>&) override;
    virtual void handleReassociationResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>&) override;
    virtual void handleDisassociationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>&) override;
    virtual void handleBeaconFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleProbeRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleProbeResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;

  public:
    virtual ~Ieee80211MgmtMesh();
    virtual void prepareManagementTransmission(Packet *packet, simtime_t ifs,
            const physicallayer::IIeee80211Mode *mode) override;
    virtual void managementTransmissionComplete(const Packet *packet) override;
    virtual void sendHwmp(const Ptr<Ieee80211HwmpFrame>& body, MacAddress receiver) override;
    virtual bool isBeaconPending() const override { return pendingBeaconTreeId >= 0; }
    virtual bool isMeshManagementActive() const override { return operationalState == OPERATING; }
    void openPeer(MacAddress peer);
    void closePeer(MacAddress peer);
    PeerState getPeerState(MacAddress peer) const;
    uint16_t getLocalLinkId(MacAddress peer) const;
    uint16_t getPeerLinkId(MacAddress peer) const;
    uint16_t getLocalAid(MacAddress peer) const;
    uint16_t getPeerAid(MacAddress peer) const;
    simtime_t getPeerDeadline(MacAddress peer) const;
    bool getNeighborOffset(MacAddress peer, int64_t& offset) const;
    uint64_t getTsfAt(simtime_t time) const;
    virtual simtime_t getNextTbtt() const override;
    simtime_t getPendingClockDrift() const { return maxClockDrift; }
};

} // namespace ieee80211
} // namespace inet

#endif
