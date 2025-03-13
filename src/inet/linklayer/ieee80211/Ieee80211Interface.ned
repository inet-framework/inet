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
// Implements a comprehensive IEEE 802.11 wireless network interface supporting
// multiple standards (a/b/g/n/p/ac) and operational modes (infrastructure, ad hoc).
// This module provides a highly modular and configurable implementation of the
// IEEE 802.11 protocol family for wireless local area networks (WLANs).
//
// The interface integrates multiple components that work together to provide
// complete WLAN functionality:
//
// - MAC: Implements medium access control with support for DCF (Distributed
//   Coordination Function) and optional QoS extensions (EDCA). The MAC handles
//   frame transmission/reception, acknowledgments, retransmissions, and channel
//   access according to the CSMA/CA mechanism.
//
// - Management: Controls the operational state of the interface through management
//   frames. Different management modules support different roles:
//   * ~Ieee80211MgmtSta for station mode (client)
//   * ~Ieee80211MgmtAp for access point mode
//   * ~Ieee80211MgmtAdhoc for ad hoc networks
//
// - Agent: Provides higher-level control of the management functions, such as
//   initiating scanning, authentication, and association processes.
//
// - LLC: Handles logical link control, adding/removing LLC headers for
//   communication with upper layers. Supports both EPD (Ethertype Protocol
//   Discrimination) and LPD (LLC Protocol Discrimination) modes.
//
// - Radio: Provides physical layer functionality with multiple implementation
//   options ranging from simple unit disk models to sophisticated models with
//   detailed signal representation and propagation characteristics.
//
// - QoS Classifier: Assigns appropriate priority levels to packets when
//   quality of service is enabled.
//
// The module is highly customizable through parameter settings and submodule
// type assignments, making it suitable for a wide range of simulation scenarios
// from simple connectivity studies to detailed protocol analysis.
//
// @see ~Ieee80211Mac, ~IIeee80211Mgmt, ~IRadio
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
        radio.signalAnalogRepresentation = default("scalar");
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
        radio: <default("Ieee80211Radio")> like IRadio {
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
