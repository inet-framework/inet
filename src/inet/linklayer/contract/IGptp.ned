//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.contract;

import inet.applications.contract.IApp;

//
// Defines the interface for IEEE 802.1AS Generalized Precision Time Protocol (gPTP)
// implementations. gPTP provides precise time synchronization for time-sensitive
// networking applications in Ethernet networks.
//
// The gPTP protocol enables network devices to synchronize their clocks with
// sub-microsecond precision, which is essential for applications requiring
// deterministic communication such as:
// - Industrial automation and control systems
// - Professional audio/video systems
// - Automotive in-vehicle networks
// - Power grid synchronization
//
// Modules implementing this interface can operate in different roles within
// a time synchronization domain, as specified by the gptpNodeType parameter:
// - MASTER_NODE: Provides the reference time for the network
// - BRIDGE_NODE: Forwards and processes time synchronization messages
// - SLAVE_NODE: Synchronizes its local clock to the master clock
//
// @see ~Gptp, ~GptpMaster, ~GptpSlave, ~GptpBridge, ~MultiDomainGptp
//
moduleinterface IGptp extends IApp
{
    parameters:
        string gptpNodeType;
        @display("i=block/app");
}
