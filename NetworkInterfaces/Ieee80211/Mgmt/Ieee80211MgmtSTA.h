//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef IEEE80211_MGMT_STA_H
#define IEEE80211_MGMT_STA_H

#include <omnetpp.h>
#include "Ieee80211MgmtBase.h"
#include "NotificationBoard.h"
#include "Ieee80211Primitives_m.h"


/**
 * Used in 802.11 infrastructure mode: handles management frames for
 * a station (STA). See corresponding NED file for a detailed description.
 *
 * @author Andras Varga
 */
class INET_API Ieee80211MgmtSTA : public Ieee80211MgmtBase
{
  public:
    //
    // Encapsulates information about the ongoing scanning process
    //
    struct ScanningInfo
    {
        MACAddress bssid; // specific BSSID to scan for, or the broadcast address
        std::string ssid; // SSID to scan for (empty=any)
        bool activeScan;  // whether to perform active or passive scanning
        double probeDelay; // delay (in s) to be used prior to transmitting a Probe frame during active scanning
        std::vector<int> channelList; // list of channels to scan
        int currentChannelIndex; // index into channelList[]
        bool busyChannelDetected; // during minChannelTime, we have to listen for busy channel
        double minChannelTime; // minimum time to spend on each channel when scanning
        double maxChannelTime; // maximum time to spend on each channel when scanning
    };

    //
    // Stores AP info received during scanning
    //
    struct APInfo
    {
        int channel;
        MACAddress address; // alias bssid
        std::string ssid;
        Ieee80211SupportedRatesElement supportedRates;
        double beaconInterval;
        double rxPower;

        bool isAuthenticated;
        int authSeqExpected;  // valid while authenticating; values: 1,3,5...
        cMessage *authTimeoutMsg; // if non-NULL: authentication is in progress

        APInfo() {
            channel=-1; beaconInterval=rxPower=0; authSeqExpected=-1;
            isAuthenticated=false; authTimeoutMsg=NULL;
        }
    };

    //
    // Associated AP, plus data associated with the association with the associated AP
    //
    struct AssociatedAPInfo : public APInfo
    {
        int receiveSequence;
        cMessage *beaconTimeoutMsg;

        AssociatedAPInfo() : APInfo() {receiveSequence=0; beaconTimeoutMsg=NULL;}
    };

  protected:
    NotificationBoard *nb;

    // number of channels in ChannelControl -- used if we're told to scan "all" channels
    int numChannels;

    // scanning status
    bool isScanning;
    ScanningInfo scanning;

    // APInfo list: we collect scanning results and keep track of ongoing authentications here
    // Note: there can be several ongoing authentications simultaneously
    typedef std::list<APInfo> AccessPointList;
    AccessPointList apList;

    // associated Access Point
    bool isAssociated;
    cMessage *assocTimeoutMsg; // if non-NULL: association is in progress
    AssociatedAPInfo assocAP;

  protected:
    virtual int numInitStages() const {return 2;}
    virtual void initialize(int);

    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleTimer(cMessage *msg);

    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleUpperMessage(cMessage *msg);

    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleCommand(int msgkind, cPolymorphic *ctrl);

    /** Utility function for handleUpperMessage() */
    virtual Ieee80211DataFrame *encapsulate(cMessage *msg);

    /** Utility function: sends authentication request */
    virtual void startAuthentication(APInfo *ap, double timeout);

    /** Utility function: sends association request */
    virtual void startAssociation(APInfo *ap, double timeout);

    /** Utility function: looks up AP in our AP list. Returns NULL if not found. */
    APInfo *lookupAP(const MACAddress& address);

    /** Utility function: clear the AP list, and cancel any pending authentications. */
    void clearAPList();

    /** Utility function: switches to the given radio channel. */
    void changeChannel(int channelNum);

    /** Stores AP info received in a beacon or probe response */
    void storeAPInfo(const MACAddress& address, const Ieee80211BeaconFrameBody& body);

    /** Switches to the next channel to scan; returns true if done (there wasn't any more channel to scan). */
    bool scanNextChannel();

    /** Broadcasts a Probe Request */
    virtual void sendProbeRequest();

    /** Missed a few consecutive beacons */
    virtual void beaconLost();

    /** Sends back result of scanning to the agent */
    void sendScanConfirm();

    /** Sends back result of authentication to the agent */
    void sendAuthenticationConfirm(APInfo *ap, int resultCode);

    /** Sends back result of association to the agent */
    void sendAssociationConfirm(APInfo *ap, int resultCode);

    /** Utility function: Cancel the existing association */
    void disassociate();

    /** Utility function: sends a confirmation to the agent */
    void sendConfirm(Ieee80211PrimConfirm *confirm, int resultCode);

    /** Utility function: sends a management frame */
    void sendManagementFrame(Ieee80211ManagementFrame *frame, const MACAddress& address);

    /** Called by the NotificationBoard whenever a change occurs we're interested in */
    virtual void receiveChangeNotification(int category, cPolymorphic *details);

    /** Utility function: converts Ieee80211StatusCode (->frame) to Ieee80211PrimResultCode (->primitive) */
    virtual int statusCodeToPrimResultCode(int statusCode);

    /** @name Processing of different frame types */
    //@{
    virtual void handleDataFrame(Ieee80211DataFrame *frame);
    virtual void handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame);
    virtual void handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame);
    virtual void handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame);
    virtual void handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame);
    virtual void handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame);
    virtual void handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame);
    virtual void handleDisassociationFrame(Ieee80211DisassociationFrame *frame);
    virtual void handleBeaconFrame(Ieee80211BeaconFrame *frame);
    virtual void handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame);
    virtual void handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame);
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

#endif


