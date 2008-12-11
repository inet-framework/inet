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

#ifndef IEEE80211_MGMT_AP_H
#define IEEE80211_MGMT_AP_H

#include <omnetpp.h>
#include <map>
#include "Ieee80211MgmtAPBase.h"
#include "NotificationBoard.h"


/**
 * Used in 802.11 infrastructure mode: handles management frames for
 * an access point (AP). See corresponding NED file for a detailed description.
 *
 * @author Andras Varga
 */
class INET_API Ieee80211MgmtAP : public Ieee80211MgmtAPBase
{
  public:
    /** State of a STA */
    enum STAStatus {NOT_AUTHENTICATED, AUTHENTICATED, ASSOCIATED};

    /** Describes a STA */
    struct STAInfo {
        MACAddress address;
        STAStatus status;
        int authSeqExpected;  // when NOT_AUTHENTICATED: transaction sequence number of next expected auth frame
        //int consecFailedTrans;  //XXX
        //double expiry;          //XXX association should expire after a while if STA is silent?
    };

    struct MAC_compare {
        bool operator()(const MACAddress& u1, const MACAddress& u2) const {return u1.compareTo(u2) < 0;}
    };
    typedef std::map<MACAddress,STAInfo, MAC_compare> STAList;

  protected:
    // configuration
    std::string ssid;
    int channelNumber;
    simtime_t beaconInterval;
    int numAuthSteps;
    Ieee80211SupportedRatesElement supportedRates;

    // state
    STAList staList; ///< list of STAs
    cMessage *beaconTimer;

  protected:
    virtual int numInitStages() const {return 2;}
    virtual void initialize(int);

    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleTimer(cMessage *msg);

    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleUpperMessage(cPacket *msg);

    /** Implements abstract Ieee80211MgmtBase method -- throws an error (no commands supported) */
    virtual void handleCommand(int msgkind, cPolymorphic *ctrl);

    /** Called by the NotificationBoard whenever a change occurs we're interested in */
    virtual void receiveChangeNotification(int category, const cPolymorphic *details);

    /** Utility function: return sender STA's entry from our STA list, or NULL if not in there */
    virtual STAInfo *lookupSenderSTA(Ieee80211ManagementFrame *frame);

    /** Utility function: set fields in the given frame and send it out to the address */
    virtual void sendManagementFrame(Ieee80211ManagementFrame *frame, const MACAddress& destAddr);

    /** Utility function: creates and sends a beacon frame */
    virtual void sendBeacon();

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
};

#endif

