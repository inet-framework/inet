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

import inet.common.IHook;
import inet.linklayer.IWirelessNic;
import inet.linklayer.ieee80211.mac.Ieee80211Mac;
import inet.linklayer.ieee80211.mgmt.IIeee80211Mgmt;
import inet.linklayer.ieee80211.mgmt.Ieee80211AgentSTA;
import inet.physicallayer.contract.IRadio;


//
// This NIC implements an 802.11 network interface card.
// It can be configured via the mgmtType parameter to act
// as an AP or a STA, or for ad-hoc mode.
//
// Potential mgmType values: Ieee80211MgmtSTASimplified, Ieee80211MgmtSTA
// Ieee80211MgmtAP, Ieee80211MgmtAPSimplified, Ieee80211MgmtAdhoc
//
module Ieee80211Nic like IWirelessNic
{
    parameters:
        string interfaceTableModule;
        string energySourceModule = default("");
        string mgmtType = default("Ieee80211MgmtSTA"); // name of the management module type (implements IIeee80211Mgmt)
        string radioType = default("Ieee80211Radio");
        string opMode @enum("b","g","a","p") = default("g");
        double bitrate @unit("bps") = default(opMode == "b" ? 11Mbps : opMode == "p" ? 27Mbps : 54Mbps);
        int numOutputHooks = default(0);
        int numInputHooks = default(0);
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
        outputHook[numOutputHooks]: <default("Nop")> like IHook if numOutputHooks>0 {
            @display("p=218,70");
        }
        inputHook[numInputHooks]: <default("Nop")> like IHook if numInputHooks>0 {
            @display("p=107,55");
        }
        // optional agent module (can be either 0 or 1 sized vector)
        agent: Ieee80211AgentSTA if _agentNeeded {
            parameters:
                @display("p=261,131");
        }
        mgmt: <mgmtType> like IIeee80211Mgmt {
            parameters:
                @display("p=157,131;q=wlanDataQueue");
        }
        mac: Ieee80211Mac {
            parameters:
                queueModule = "mgmt";
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

        mgmt.upperLayerOut --> { @display("m=n"); } --> upperLayerOut if numInputHooks == 0;
        mgmt.upperLayerIn <-- { @display("m=n"); } <-- upperLayerIn if numOutputHooks == 0;

        mgmt.upperLayerOut --> inputHook[0].in if numInputHooks > 0;
        for i=0..numInputHooks-2 {
            inputHook[i].out --> inputHook[i+1].in;
        }
        inputHook[numInputHooks-1].out --> { @display("m=n"); } --> upperLayerOut if numInputHooks > 0;
        outputHook[0].in <-- { @display("m=n"); } <-- upperLayerIn if numOutputHooks > 0;
        for i=0..numOutputHooks-2 {
            outputHook[i].out --> outputHook[i+1].in;
        }
        mgmt.upperLayerIn <-- outputHook[numOutputHooks-1].out if numOutputHooks > 0;
}

