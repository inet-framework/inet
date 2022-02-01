//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211MGMTSTA_H
#define __INET_IEEE80211MGMTSTA_H

#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211Primitives_m.h"

namespace inet {

class NetworkInterface;

namespace ieee80211 {

/**
 * Used in 802.11 infrastructure mode: handles management frames for
 * a station (STA). See corresponding NED file for a detailed description.
 *
 */
class INET_API Ieee80211MgmtSta : public Ieee80211MgmtBase, protected cListener
{
  public:
    //
    // Encapsulates information about the ongoing scanning process
    //
    struct ScanningInfo {
        MacAddress bssid; // specific BSSID to scan for, or the broadcast address
        std::string ssid; // SSID to scan for (empty=any)
        bool activeScan; // whether to perform active or passive scanning
        simtime_t probeDelay; // delay (in s) to be used prior to transmitting a Probe frame during active scanning
        std::vector<int> channelList; // list of channels to scan
        int currentChannelIndex; // index into channelList[]
        bool busyChannelDetected; // during minChannelTime, we have to listen for busy channel
        simtime_t minChannelTime; // minimum time to spend on each channel when scanning
        simtime_t maxChannelTime; // maximum time to spend on each channel when scanning
    };

    //
    // Stores AP info received during scanning
    //
    struct ApInfo : public cObject {
        int channel;
        MacAddress address; // alias bssid
        std::string ssid;
        Ieee80211SupportedRatesElement supportedRates;
        simtime_t beaconInterval;
        double rxPower;

        bool isAuthenticated;
        int authSeqExpected; // valid while authenticating; values: 1,3,5...
        cMessage *authTimeoutMsg; // if non-nullptr: authentication is in progress

        ApInfo()
        {
            channel = -1;
            beaconInterval = rxPower = 0;
            authSeqExpected = -1;
            isAuthenticated = false;
            authTimeoutMsg = nullptr;
        }
    };

    //
    // Associated AP, plus data associated with the association with the associated AP
    //
    struct AssociatedApInfo : public ApInfo {
        int receiveSequence;
        cMessage *beaconTimeoutMsg;

        AssociatedApInfo() : ApInfo() { receiveSequence = 0; beaconTimeoutMsg = nullptr; }
    };

  protected:
    cModule *host;

    // number of channels in RadioMedium -- used if we're told to scan "all" channels
    int numChannels;

    // scanning status
    bool isScanning;
    ScanningInfo scanning;

    // ApInfo list: we collect scanning results and keep track of ongoing authentications here
    // Note: there can be several ongoing authentications simultaneously
    typedef std::list<ApInfo> AccessPointList;
    AccessPointList apList;

    // associated Access Point
    cMessage *assocTimeoutMsg; // if non-nullptr: association is in progress
    AssociatedApInfo assocAP;

  public:
    Ieee80211MgmtSta() : host(nullptr), numChannels(-1), isScanning(false), assocTimeoutMsg(nullptr) {}

    virtual const ApInfo *getAssociatedAp() { return &assocAP; }

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleTimer(cMessage *msg) override;

    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleCommand(int msgkind, cObject *ctrl) override;

    /** Utility function: sends authentication request */
    virtual void startAuthentication(ApInfo *ap, simtime_t timeout);

    /** Utility function: sends association request */
    virtual void startAssociation(ApInfo *ap, simtime_t timeout);

    /** Utility function: looks up AP in our AP list. Returns nullptr if not found. */
    virtual ApInfo *lookupAP(const MacAddress& address);

    /** Utility function: clear the AP list, and cancel any pending authentications. */
    virtual void clearAPList();

    /** Utility function: switches to the given radio channel. */
    virtual void changeChannel(int channelNum);

    /** Stores AP info received in a beacon or probe response */
    virtual void storeAPInfo(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header, const Ptr<const Ieee80211BeaconFrame>& body);

    /** Switches to the next channel to scan; returns true if done (there wasn't any more channel to scan). */
    virtual bool scanNextChannel();

    /** Broadcasts a Probe Request */
    virtual void sendProbeRequest();

    /** Missed a few consecutive beacons */
    virtual void beaconLost();

    /** Sends back result of scanning to the agent */
    virtual void sendScanConfirm();

    /** Sends back result of authentication to the agent */
    virtual void sendAuthenticationConfirm(ApInfo *ap, Ieee80211PrimResultCode resultCode);

    /** Sends back result of association to the agent */
    virtual void sendAssociationConfirm(ApInfo *ap, Ieee80211PrimResultCode resultCode);

    /** Utility function: Cancel the existing association */
    virtual void disassociate();

    /** Utility function: sends a confirmation to the agent */
    virtual void sendConfirm(Ieee80211PrimConfirm *confirm, Ieee80211PrimResultCode resultCode);

    /** Utility function: sends a management frame */
    virtual void sendManagementFrame(const char *name, const Ptr<Ieee80211MgmtFrame>& body, int subtype, const MacAddress& address);

    /** Called by the signal handler whenever a change occurs we're interested in */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details) override;

    /** Utility function: converts Ieee80211StatusCode (->frame) to Ieee80211PrimResultCode (->primitive) */
    virtual Ieee80211PrimResultCode statusCodeToPrimResultCode(int statusCode);

    /** @name Processing of different frame types */
    //@{
    virtual void handleAuthenticationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleDeauthenticationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleAssociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleAssociationResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleReassociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleReassociationResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleDisassociationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleBeaconFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleProbeRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleProbeResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    //@}

    /** @name Processing of different agent commands */
    //@{
    virtual void processScanCommand(Ieee80211Prim_ScanRequest *ctrl);
    virtual void processAuthenticateCommand(Ieee80211Prim_AuthenticateRequest *ctrl);
    virtual void processDeauthenticateCommand(Ieee80211Prim_DeauthenticateRequest *ctrl);
    virtual void processAssociateCommand(Ieee80211Prim_AssociateRequest *ctrl);
    virtual void processReassociateCommand(Ieee80211Prim_ReassociateRequest *ctrl);
    virtual void processDisassociateCommand(Ieee80211Prim_DisassociateRequest *ctrl);
    //@}
};

} // namespace ieee80211

} // namespace inet

#endif

