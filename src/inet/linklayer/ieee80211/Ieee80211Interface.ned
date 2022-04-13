//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211;

import inet.linklayer.common.IIeee8021dQosClassifier;
import inet.linklayer.contract.IWirelessInterface;
import inet.linklayer.ieee80211.llc.IIeee80211Llc;
import inet.linklayer.ieee80211.mgmt.IIeee80211Agent;
import inet.linklayer.ieee80211.mgmt.IIeee80211Mgmt;
import inet.linklayer.ieee80211.mib.Ieee80211Mib;
import inet.networklayer.common.NetworkInterface;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadio;

//
// This module implements an IEEE 802.11 network interface.
//
// It can be configured via the mgmt.typename parameter to act
// as an AP or a STA, or for ad-hoc mode.
//
// Potential mgmType values: ~Ieee80211MgmtStaSimplified, ~Ieee80211MgmtSta,
// ~Ieee80211MgmtAp, ~Ieee80211MgmtApSimplified, ~Ieee80211MgmtAdhoc.
//
// A classifier is needed if you want to use QoS.
//
module Ieee80211Interface extends NetworkInterface like IWirelessInterface
{
    parameters:
        string interfaceTableModule;
        string energySourceModule = default("");
        string opMode @enum("a","b","g(erp)","g(mixed)","n(mixed-2.4Ghz)","p","ac") = default("g(mixed)");
        string address @mutable = default("auto"); // MAC address as hex string (12 hex digits), or
                                                   // "auto". "auto" values will be replaced by
                                                   // a generated MAC address in init stage 0.
        string protocol = default("");
        double bitrate @unit(bps) = default(-1bps);
        **.opMode = this.opMode;
        **.bitrate = this.bitrate;
        mac.modeSet = default(this.opMode);
        mac.*.rateSelection.dataFrameBitrate = default(this.bitrate);
        *.macModule = default(absPath(".mac"));
        *.mibModule = default(absPath(".mib"));
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        *.energySourceModule = default(absPath(this.energySourceModule));
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input radioIn @labels(IWirelessSignal);
    submodules:
        mib: Ieee80211Mib {
            parameters:
                @display("p=100,300;is=s");
        }
        llc: <default(opMode == "p" ? "Ieee80211LlcEpd" : "Ieee80211LlcLpd")> like IIeee80211Llc {
            parameters:
                @display("p=300,200");
        }
        classifier: <default("OmittedIeee8021dQosClassifier")> like IIeee8021dQosClassifier {
            parameters:
                @display("p=500,100");
        }
        agent: <default("Ieee80211AgentSta")> like IIeee80211Agent if typename != "" {
            parameters:
                @display("p=700,300");
        }
        mgmt: <default("Ieee80211MgmtSta")> like IIeee80211Mgmt {
            parameters:
                @display("p=500,300");
        }
        mac: <default("Ieee80211Mac")> like IIeee80211Mac {
            parameters:
                @display("p=300,300");
        }
        radio: <default("Ieee80211ScalarRadio")> like IRadio {
            parameters:
                @display("p=300,400");
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
        llc.upperLayerIn <-- { @display("m=n"); } <-- classifier.out;

        llc.lowerLayerOut --> mac.upperLayerIn;
        llc.lowerLayerIn <-- mac.upperLayerOut;

        classifier.in <-- { @display("m=n"); } <-- upperLayerIn;
}

