//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IEEE80211MGMTSTA_H
#define __INET_IEEE80211MGMTSTA_H

#include "inet/common/INETDefs.h"

#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211Primitives_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

class InterfaceEntry;

namespace ieee80211 {

/**
 * Used in 802.11 infrastructure mode: handles management frames for
 * a station (STA). See corresponding NED file for a detailed description.
 *
 * @author Andras Varga
 */
class INET_API Ieee80211MgmtSTA : public Ieee80211MgmtBase, protected cListener
{
  public:
    //
    // Encapsulates information about the ongoing scanning process
    //
    struct ScanningInfo
    {
        MACAddress bssid;    // specific BSSID to scan for, or the broadcast address
        std::string ssid;    // SSID to scan for (empty=any)
        bool activeScan;    // whether to perform active or passive scanning
        simtime_t probeDelay;    // delay (in s) to be used prior to transmitting a Probe frame during active scanning
        std::vector<int> channelList;    // list of channels to scan
        int currentChannelIndex;    // index into channelList[]
        bool busyChannelDetected;    // during minChannelTime, we have to listen for busy channel
        simtime_t minChannelTime;    // minimum time to spend on each channel when scanning
        simtime_t maxChannelTime;    // maximum time to spend on each channel when scanning
    };

    //
    // Stores AP info received during scanning
    //
    struct APInfo
    {
        int channel;
        MACAddress address;    // alias bssid
        std::string ssid;
        Ieee80211SupportedRatesElement supportedRates;
        simtime_t beaconInterval;
        double rxPower;

        bool isAuthenticated;
        int authSeqExpected;    // valid while authenticating; values: 1,3,5...
        cMessage *authTimeoutMsg;    // if non-nullptr: authentication is in progress

        APInfo()
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
    struct AssociatedAPInfo : public APInfo
    {
        int receiveSequence;
        cMessage *beaconTimeoutMsg;

        AssociatedAPInfo() : APInfo() { receiveSequence = 0; beaconTimeoutMsg = nullptr; }
    };

  protected:
    cModule *host;
    IInterfaceTable *interfaceTable;
    InterfaceEntry *myIface;

    // number of channels in RadioMedium -- used if we're told to scan "all" channels
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
    cMessage *assocTimeoutMsg;    // if non-nullptr: association is in progress
    AssociatedAPInfo assocAP;

  public:
    Ieee80211MgmtSTA() : host(nullptr), interfaceTable(nullptr), myIface(nullptr), numChannels(-1), isScanning(false), isAssociated(false), assocTimeoutMsg(nullptr) {}

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleTimer(cMessage *msg) override;

    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleUpperMessage(cPacket *msg) override;

    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleCommand(int msgkind, cObject *ctrl) override;

    /** Utility function for handleUpperMessage() */
    virtual Ieee80211DataFrame *encapsulate(cPacket *msg);

    /** Utility method to decapsulate a data frame */
    virtual cPacket *decapsulate(Ieee80211DataFrame *frame);

    /** Utility function: sends authentication request */
    virtual void startAuthentication(APInfo *ap, simtime_t timeout);

    /** Utility function: sends association request */
    virtual void startAssociation(APInfo *ap, simtime_t timeout);

    /** Utility function: looks up AP in our AP list. Returns nullptr if not found. */
    virtual APInfo *lookupAP(const MACAddress& address);

    /** Utility function: clear the AP list, and cancel any pending authentications. */
    virtual void clearAPList();

    /** Utility function: switches to the given radio channel. */
    virtual void changeChannel(int channelNum);

    /** Stores AP info received in a beacon or probe response */
    virtual void storeAPInfo(const MACAddress& address, const Ieee80211BeaconFrameBody& body);

    /** Switches to the next channel to scan; returns true if done (there wasn't any more channel to scan). */
    virtual bool scanNextChannel();

    /** Broadcasts a Probe Request */
    virtual void sendProbeRequest();

    /** Missed a few consecutive beacons */
    virtual void beaconLost();

    /** Sends back result of scanning to the agent */
    virtual void sendScanConfirm();

    /** Sends back result of authentication to the agent */
    virtual void sendAuthenticationConfirm(APInfo *ap, int resultCode);

    /** Sends back result of association to the agent */
    virtual void sendAssociationConfirm(APInfo *ap, int resultCode);

    /** Utility function: Cancel the existing association */
    virtual void disassociate();

    /** Utility function: sends a confirmation to the agent */
    virtual void sendConfirm(Ieee80211PrimConfirm *confirm, int resultCode);

    /** Utility function: sends a management frame */
    virtual void sendManagementFrame(Ieee80211ManagementFrame *frame, const MACAddress& address);

    /** Called by the signal handler whenever a change occurs we're interested in */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, long value DETAILS_ARG) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;

    /** Utility function: converts Ieee80211StatusCode (->frame) to Ieee80211PrimResultCode (->primitive) */
    virtual int statusCodeToPrimResultCode(int statusCode);

    /** @name Processing of different frame types */
    //@{
    virtual void handleDataFrame(Ieee80211DataFrame *frame) override;
    virtual void handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame) override;
    virtual void handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame) override;
    virtual void handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame) override;
    virtual void handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame) override;
    virtual void handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame) override;
    virtual void handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame) override;
    virtual void handleDisassociationFrame(Ieee80211DisassociationFrame *frame) override;
    virtual void handleBeaconFrame(Ieee80211BeaconFrame *frame) override;
    virtual void handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame) override;
    virtual void handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame) override;
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

#endif // ifndef __INET_IEEE80211MGMTSTA_H

