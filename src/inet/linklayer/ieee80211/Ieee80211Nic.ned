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

import inet.linklayer.common.IIeee8021dQoSClassifier;
import inet.linklayer.contract.IWirelessNic;
import inet.linklayer.ieee80211.mgmt.IIeee80211Mgmt;
import inet.linklayer.ieee80211.mgmt.Ieee80211AgentSTA;
import inet.physicallayer.contract.packetlevel.IRadio;


//
// This NIC implements an 802.11 network interface card.
// It can be configured via the mgmtType parameter to act
// as an AP or a STA, or for ad-hoc mode.
//
// Potential mgmType values: Ieee80211MgmtSTASimplified, Ieee80211MgmtSTA
// Ieee80211MgmtAP, Ieee80211MgmtAPSimplified, Ieee80211MgmtAdhoc.
//
// A classifier is needed if you want to use QoS.
//
module Ieee80211Nic like IWirelessNic
{
    parameters:
        string interfaceTableModule;
        string energySourceModule = default("");
        string classifierType = default(""); // needed for EDCA, as the classifier assigns User Priority (UP) to packets; UP determines Access Category via TID
        string mgmtType = default("Ieee80211MgmtSTA"); // name of the management module type (implements IIeee80211Mgmt)
        string radioType = default("Ieee80211ScalarRadio");
        string macType = default("Ieee80211CompatibleMac");   // choose from Ieee80211OldMac, Ieee80211Mac, Ieee80211CompatibleMac
        string opMode @enum("a","b","g","n","p") = default("g");
        double bitrate @unit("bps") = default(opMode == "b" ? 11Mbps : opMode == "p" ? 27Mbps : 54Mbps);
        bool _agentNeeded = (mgmtType == "Ieee80211MgmtSTA");   // internal par. do not use, shows if optional agent module is needed
        **.opMode = opMode;
        **.bitrate = bitrate;
        @display("i=block/ifcard;bgb=336,357");
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.energySourceModule = default(absPath(energySourceModule));
    gates:
        input upperLayerIn; // to upper layers
        output upperLayerOut; // from upper layers
        input radioIn @labels(IRadioFrame); // to receive radio frames
    submodules:
        classifier: <classifierType> like IIeee8021dQoSClassifier if classifierType != "" {
            @display("p=110,59;i=block/classifier");
        }
        agent: Ieee80211AgentSTA if _agentNeeded {
            parameters:
                @display("p=261,131");
        }
        mgmt: <mgmtType> like IIeee80211Mgmt {
            parameters:
                macModule = "^.mac";
                @display("p=157,131");
        }
        mac: <macType> like IIeee80211Mac {
            parameters:
                @display("p=157,217");
        }
        radio: <radioType> like IRadio {
            parameters:
                @display("p=157,302");
        }
    connections:
        radioIn --> { @display("m=s"); } --> radio.radioIn;
        radio.upperLayerIn <-- mac.lowerLayerOut;
        radio.upperLayerOut --> mac.lowerLayerIn;

        mac.upperLayerOut --> mgmt.macIn;
        mac.upperLayerIn <-- mgmt.macOut;

        mgmt.agentOut --> agent.mgmtIn if _agentNeeded;
        mgmt.agentIn <-- agent.mgmtOut if _agentNeeded;

        mgmt.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
        mgmt.upperLayerIn <-- { @display("m=n"); } <-- upperLayerIn if classifierType == "";
        mgmt.upperLayerIn <-- { @display("m=n"); } <-- classifier.out if classifierType != "";
        classifier.in <-- { @display("m=n"); } <-- upperLayerIn if classifierType != "";
}

