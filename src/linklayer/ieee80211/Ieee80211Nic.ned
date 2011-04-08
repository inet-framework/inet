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


package inet.linklayer.ieee80211;

import inet.linklayer.ieee80211.radio.Ieee80211Radio;
import inet.linklayer.ieee80211.mgmt.Ieee80211AgentSTA;
import inet.linklayer.ieee80211.mgmt.IIeee80211Mgmt;
import inet.linklayer.ieee80211.mac.Ieee80211Mac;
import inet.linklayer.IWirelessNic;

//
// This NIC implements an 802.11 network interface card.
// It can be configured via the mgmtType parameter to act
// as an AP or a STA, or for ad-hoc mode. By default it is 
// configured for a simplified station mode (no channel scanning
// association, authentication etc.)
//
// Potential mgmType values: Ieee80211MgmtSTASimplified, Ieee80211MgmtSTA
// Ieee80211MgmtAP, Ieee80211MgmtAPSimplified, Ieee80211MgmtAdhoc
//
module Ieee80211Nic like IWirelessNic
{
    parameters:
        string mgmtType = default("Ieee80211MgmtSTA"); // name of the management module type (implements IIeee80211Mgmt)
        bool _agentNeeded = (mgmtType == "Ieee80211MgmtSTA");   // internal par. do not use, shows if optional agent module is needed 
        @display("i=block/ifcard;bgb=259,320");
    gates:
        input uppergateIn; // to upper layers
        output uppergateOut; // from upper layers
        input radioIn @labels(AirFrame); // to receive AirFrames
    submodules:
        // optional agent module (can be either 0 or 1 sized vector)
        agent: Ieee80211AgentSTA if _agentNeeded {
            parameters:
                @display("p=202,69");
        }
        mgmt: <mgmtType> like IIeee80211Mgmt {
            parameters:
                @display("p=96,69;q=wlanDataQueue");
        }
        mac: Ieee80211Mac {
            parameters:
                queueModule = "mgmt";
                @display("p=96,155");
        }
        radio: Ieee80211Radio {
            parameters:
                @display("p=96,240");
        }
    connections:
        radioIn --> radio.radioIn;
        radio.uppergateIn <-- mac.lowergateOut;
        radio.uppergateOut --> mac.lowergateIn;

        mac.uppergateOut --> mgmt.macIn;
        mac.uppergateIn <-- mgmt.macOut;

        mgmt.uppergateOut --> uppergateOut;
        mgmt.uppergateIn <-- uppergateIn;
        
        mgmt.agentOut --> agent.mgmtIn if _agentNeeded;
        mgmt.agentIn <-- agent.mgmtOut if _agentNeeded;
}

