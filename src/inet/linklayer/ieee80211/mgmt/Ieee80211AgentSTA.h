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

#ifndef __INET_IEEE80211AGENTSTA_H
#define __INET_IEEE80211AGENTSTA_H

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/linklayer/ieee80211/mgmt/Ieee80211Primitives_m.h"
#include "inet/networklayer/common/InterfaceTable.h"

namespace inet {

namespace ieee80211 {

/**
 * Used in 802.11 infrastructure mode: in a station (STA), this module
 * controls channel scanning, association and handovers, by sending commands
 * (e.g. Ieee80211Prim_ScanRequest) to the management module (Ieee80211MgmtSTA).
 *
 * See corresponding NED file for a detailed description.
 *
 * @author Andras Varga
 */
class INET_API Ieee80211AgentSTA : public cSimpleModule, public cListener
{
  protected:
    InterfaceEntry *myIface;
    MACAddress prevAP;
    bool activeScan;
    std::vector<int> channelsToScan;
    simtime_t probeDelay;
    simtime_t minChannelTime;
    simtime_t maxChannelTime;
    simtime_t authenticationTimeout;
    simtime_t associationTimeout;

    std::string default_ssid;

    //Statistics:
    static simsignal_t sentRequestSignal;
    static simsignal_t acceptConfirmSignal;
    static simsignal_t dropConfirmSignal;

  public:
    Ieee80211AgentSTA() : myIface(NULL) {}

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int);

    /** Overridden cSimpleModule method */
    virtual void handleMessage(cMessage *msg);

    /** Handle timers */
    virtual void handleTimer(cMessage *msg);

    /** Handle responses from mgmgt */
    virtual void handleResponse(cMessage *msg);

    /** Redefined from cListener; called by signal handler */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);

    // utility method: attaches object to a message as controlInfo, and sends it to mgmt
    virtual void sendRequest(Ieee80211PrimRequest *req);

    /** Sending of Request primitives */
    //@{
    virtual void sendScanRequest();
    virtual void sendAuthenticateRequest(const MACAddress& address);
    virtual void sendDeauthenticateRequest(const MACAddress& address, int reasonCode);
    virtual void sendAssociateRequest(const MACAddress& address);
    virtual void sendReassociateRequest(const MACAddress& address);
    virtual void sendDisassociateRequest(const MACAddress& address, int reasonCode);
    //@}

    /** Processing Confirm primitives */
    //@{
    virtual void processScanConfirm(Ieee80211Prim_ScanConfirm *resp);
    virtual void processAuthenticateConfirm(Ieee80211Prim_AuthenticateConfirm *resp);
    virtual void processAssociateConfirm(Ieee80211Prim_AssociateConfirm *resp);
    virtual void processReassociateConfirm(Ieee80211Prim_ReassociateConfirm *resp);
    //@}

    /** Choose one AP from the list to associate with */
    virtual int chooseBSS(Ieee80211Prim_ScanConfirm *resp);

    // utility method, for debugging
    virtual void dumpAPList(Ieee80211Prim_ScanConfirm *resp);
};

} // namespace ieee80211

} // namespace inet

#endif // ifndef __INET_IEEE80211AGENTSTA_H

