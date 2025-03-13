//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mgmt;

//
// Defines the interface for IEEE 802.11 management modules that handle the
// operational aspects of wireless LANs beyond basic frame exchange. This
// interface specifies the required gates and parameters for any module
// implementing management functionality within an ~Ieee80211Interface.
//
// The IEEE 802.11 management layer is responsible for:
// - Network discovery through scanning (passive and active)
// - Authentication with access points or peer stations
// - Association/reassociation with access points
// - Handling of management frames (Beacon, Probe Request/Response, etc.)
// - Maintaining state information about the wireless network
// - Coordinating transitions between different operational states
//
// Different implementations of this interface support different operational
// modes of IEEE 802.11:
// - ~Ieee80211MgmtSta: Station mode for connecting to access points
// - ~Ieee80211MgmtAp: Access point mode for serving stations
// - ~Ieee80211MgmtAdhoc: Ad hoc mode for direct station-to-station communication
//
// The management module communicates with the MAC layer (~IIeee80211Mac) through
// management frames, and may optionally interact with an agent module that
// provides higher-level control of management operations.
//
// @see ~Ieee80211MgmtSta, ~Ieee80211MgmtAp, ~Ieee80211MgmtAdhoc, ~IIeee80211Mac
//
moduleinterface IIeee80211Mgmt
{
    parameters:
        @display("i=block/cogwheel");
        string macModule;               // The path to the MAC module
    gates:
        input macIn @labels(Ieee80211MacHeader);
        output macOut @labels(Ieee80211MacHeader);
        input agentIn;   // Optional gates used for the agent in stations
        output agentOut; // Optional gates used for the agent in stations
}
