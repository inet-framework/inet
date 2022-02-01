//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.emulation.linklayer.ieee80211;

import inet.emulation.common.ExtInterface;
import inet.emulation.linklayer.ethernet.ExtEthernetTapDevice;
import inet.linklayer.common.IIeee8021dQosClassifier;
import inet.linklayer.contract.IWirelessInterface;
import inet.linklayer.ieee80211.IIeee80211Mac;
import inet.linklayer.ieee80211.llc.IIeee80211Llc;
import inet.linklayer.ieee80211.mgmt.IIeee80211Agent;
import inet.linklayer.ieee80211.mgmt.IIeee80211Mgmt;
import inet.linklayer.ieee80211.mib.Ieee80211Mib;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadio;

//
// This module provides an IEEE 802.11 network interface suitable for emulation.
// The upper part of the network interface is realized in the real world using
// a real TAP device of the host computer which is running the simulation.
//
module ExtUpperIeee80211Interface extends ExtInterface like IWirelessInterface
{
    parameters:
        string energySourceModule = default("");
        string opMode @enum("a","b","g(erp)","g(mixed)","n(mixed-2.4Ghz)","p","ac") = default("g(mixed)");
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
        input radioIn @labels(IWirelessSignal); // to receive signals
    submodules:
        mib: Ieee80211Mib {
            parameters:
                @display("p=100,300;is=s");
        }
        tap: ExtEthernetTapDevice {
            parameters:
                @display("p=300,100");
        }
        llc: <default("Ieee80211Portal")> like IIeee80211Llc {
            parameters:
                @display("p=300,200");
        }
        classifier: <default("OmittedIeee8021dQosClassifier")> like IIeee8021dQosClassifier {
            parameters:
                @display("p=500,150");
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
    connections allowunconnected:
        radioIn --> { @display("m=s"); } --> radio.radioIn;
        radio.upperLayerIn <-- mac.lowerLayerOut;
        radio.upperLayerOut --> mac.lowerLayerIn;

        mac.mgmtOut --> mgmt.macIn;
        mac.mgmtIn <-- mgmt.macOut;

        mgmt.agentOut --> agent.mgmtIn if exists(agent);
        mgmt.agentIn <-- agent.mgmtOut if exists(agent);

        llc.upperLayerOut --> { @display("m=n"); } --> tap.lowerLayerIn;
        llc.upperLayerIn <-- { @display("m=n"); } <-- classifier.out;

        llc.lowerLayerOut --> mac.upperLayerIn;
        llc.lowerLayerIn <-- mac.upperLayerOut;

        classifier.in <-- { @display("m=n"); } <-- tap.lowerLayerOut;
}

