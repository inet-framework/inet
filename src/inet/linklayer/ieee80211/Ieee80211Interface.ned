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

import inet.linklayer.common.IIeee8021dQosClassifier;
import inet.linklayer.contract.IWirelessInterface;
import inet.linklayer.ieee80211.llc.IIeee80211Llc;
import inet.linklayer.ieee80211.mgmt.IIeee80211Agent;
import inet.linklayer.ieee80211.mgmt.IIeee80211Mgmt;
import inet.linklayer.ieee80211.mib.Ieee80211Mib;
import inet.networklayer.common.InterfaceEntry;
import inet.physicallayer.contract.packetlevel.IRadio;

//
// This NIC implements an 802.11 network interface card.
// It can be configured via the mgmt.typename parameter to act
// as an AP or a STA, or for ad-hoc mode.
//
// Potential mgmType values: ~Ieee80211MgmtStaSimplified, ~Ieee80211MgmtSta,
// ~Ieee80211MgmtAp, ~Ieee80211MgmtApSimplified, ~Ieee80211MgmtAdhoc.
//
// A classifier is needed if you want to use QoS.
//
module Ieee80211Interface extends InterfaceEntry like IWirelessInterface
{
    parameters:
        @class(::inet::InterfaceEntry);
        string interfaceTableModule;
        string energySourceModule = default("");
        string opMode @enum("a","b","g(erp)","g(mixed)","n(mixed-2.4Ghz)","p","ac") = default("g(mixed)");
        string address = default("auto"); // MAC address as hex string (12 hex digits), or
                                          // "auto". "auto" values will be replaced by
                                          // a generated MAC address in init stage 0.
        double bitrate @unit(bps) = default(-1bps);
        **.opMode = this.opMode;
        **.bitrate = this.bitrate;
        mac.modeSet = default(this.opMode);
        mac.*.rateSelection.dataFrameBitrate = default(this.bitrate);
        *.macModule = default(absPath(".mac"));
        *.mibModule = default(absPath(".mib"));
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        *.energySourceModule = default(absPath(this.energySourceModule));
        @display("i=block/ifcard");
    gates:
        input upperLayerIn; // to upper layers
        output upperLayerOut; // from upper layers
        input radioIn @labels(ISignal); // to receive signals
    submodules:
        mib: Ieee80211Mib {
            parameters:
                @display("p=100,100;is=s");
        }
        llc: <default(opMode == "p" ? "Ieee80211LlcEpd" : "Ieee80211LlcLpd")> like IIeee80211Llc {
            parameters:
                @display("p=250,200");
        }
        classifier: <default("")> like IIeee8021dQosClassifier if typename != "" {
            parameters:
                @display("p=400,100");
        }
        agent: <default("Ieee80211AgentSta")> like IIeee80211Agent if typename != "" {
            parameters:
                @display("p=550,350");
        }
        mgmt: <default("Ieee80211MgmtSta")> like IIeee80211Mgmt {
            parameters:
                @display("p=400,350");
        }
        mac: <default("Ieee80211Mac")> like IIeee80211Mac {
            parameters:
                @display("p=250,350");
        }
        radio: <default("Ieee80211ScalarRadio")> like IRadio {
            parameters:
                @display("p=250,500");
        }
    connections:
        radioIn --> { @display("m=s"); } --> radio.radioIn;
        radio.upperLayerIn <-- mac.lowerLayerOut;
        radio.upperLayerOut --> mac.lowerLayerIn;

        mac.mgmtOut --> mgmt.macIn;
        mac.mgmtIn <-- mgmt.macOut;

        mgmt.agentOut --> agent.mgmtIn if exists(agent);
        mgmt.agentIn <-- agent.mgmtOut if exists(agent);

        llc.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
        llc.upperLayerIn <-- { @display("m=n"); } <-- upperLayerIn if !exists(classifier);
        llc.upperLayerIn <-- { @display("m=n"); } <-- classifier.out if exists(classifier);

        llc.lowerLayerOut --> mac.upperLayerIn;
        llc.lowerLayerIn <-- mac.upperLayerOut;

        classifier.in <-- { @display("m=n"); } <-- upperLayerIn if exists(classifier);
}

