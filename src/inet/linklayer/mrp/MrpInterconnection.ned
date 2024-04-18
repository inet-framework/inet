//
// Copyright (C) 2024 Daniel Zeitler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.mrp;

import inet.linklayer.contract.IMediaRedundancy;
import inet.linklayer.mrp.Mrp;

//
// Implements ring interconnection support for the Media Redundancy Protocol
// (MRP) as specified in IEC 62439-2. This is an extension of the ~Mrp module
// with the Media Redundancy Interconnection Manager (MIM) and Media Redundancy
// Interconnection Client (MIC) roles. This module should be used instead of
// ~Mrp in MRP nodes that are also ring interconnection nodes.
//
// As per IEC 62439-2, it is possible to redundantly interconnect two or more
// MRP rings via the Media Redundancy Protocol (MRP) Interconnection Protocol.
// An MRP Interconnection setup consists at a minimum of two rings and two
// redundant interconnection links between these rings. The redundant
// interconnection links are provided by four dedicated devices supporting the
// MRP Interconnection protocol. The roles of these four devices must be one
// Media Redundancy Interconnection Manager (MIM) and three Media Redundancy
// Interconnection Clients (MIC). All four devices must additionally take one of
// the basic operational MRP roles, MRC or MRM, as they are also part of the MRP
// rings. The MRP Interconnect protocol may use either or ring check (RC) mode
// (MRP_InTest frames) or link check (LC) mode (IEEE802.1q CFM-based) to find
// the failure in the network among these 4 devices.
//
// To configure an MRP Interconnection in a network, follow these steps:
//
// 1. Identify the 2+2 nodes that interconnect the two rings with two direct
//    links. Ensure that the MRP components in them are of the type
//    ~MrpInterconnection and not plain ~Mrp. Using the `interconnectionID`
//    parameter, assign a unique interconnection ID to the four nodes.
//
// 2. Assign Interconnection Ports: Configure the `interconnectionPort`
//    parameter. If you already set up the network topology so the
//    interconnection ports are index 2, the default setting will suffice.
//
// 3. Set Node Roles: Use the `interconnectionRole` parameter, designate one of
//    the four interconnection nodes as the Media Redundancy Interconnection
//    Manager (MIM) and the other three as Media Redundancy Interconnection
//    Clients (MIC).
//
// 4. Adjust Features: Modify optional feature flags and settings as needed.
//
// @see ~Mrp, ~MrpRelay, ~MrpMacForwardingTable, ~L2NetworkConfigurator, ~InterfaceTable
//
simple MrpInterconnection extends Mrp like IMediaRedundancy
{
    parameters:
        @class(MrpInterconnection);

        // Specifies the interconnection role for each node within the redundancy setup:
        // Interconnection Client (MIC, value 1) or Interconnection Manager (MIM, value 2).
        // Only one of the four required nodes can assume the manager role (MIM),
        // while the other three should be clients (MIC).
        string interconnectionRole @enum("MIC", "MIM");

        // Port number of the interconnection port.
        int interconnectionPort = default(2);

        // Unique identifier for the interconnection, ranging from 0 to 65535.
        int interconnectionID = default(1);

        // Defines how to check the integrity of the interconnection.
        // - RC (Ring Check) mode: MIM sends periodic interconnection test
        //   frames on its interconnection and ring ports, which be received on
        //   the opposite port. If not received within five interconnection test
        //   intervals, the interconnection ring is assumed broken, and the MIM
        //   changes its interconnection port from BLOCKING to FORWARDING.
        // - LC (Link Check) mode: Nodes check their attached links using 802.1q-CFM
        //   (CCM messages) on the interconnection port, and report changes to the MIM.
        //   The MIM also perform a link status poll at start-up.
        // Enabling both link check and ring check is possible but not recommended.
        string interconnectionCheckMode @enum("RC", "LC", "RC_AND_LC") = default("RC");

        // Format for the text displayed above the module icon.
        // Directives: `%r`: MRP role, `%n`: node state, `%g`: ring state,
        // `%R`: interconnection role, `%N`: interconnection node state, `%I`: interconnection state.
        displayStringTextFormat = default("role: %r, %R\nstate: %n, in: %N\nring: %g in: %I");

        @display("i=block/network2");
        @signal[inTopologyChange](type="long");
        @signal[inLinkChange](type="long");
        @signal[receivedInTest](type="long");
        @signal[receivedInChange](type="unsigned long");
        @signal[inStatusPoll](type="long");
        @signal[receivedInStatusPoll](type="long");
        @signal[inTest](type="long");
        @signal[interconnectionStateChanged](type="long");
        @signal[test](type="long");
        @signal[topologyChange](type="unsigned long");
        @signal[linkChange](type="double");
        @signal[continuityCheck](type="long");
        @signal[clearFDB](type="double");
        @signal[receivedTest](type="unsigned long");
        @signal[receivedChange](type="unsigned long");
        @signal[receivedContinuityCheck](type="long");
        @signal[ringStateChanged](type="unsigned long");
        @signal[portStateChanged](type="unsigned long");
        @statistic[testFrame](title="Testframes sent"; source="test"; record=count,vector; interpolationmode=none);
        @statistic[linkChange](title="LinkChange in us"; source="linkChange"; record=count,vector; interpolationmode=none);
        @statistic[topologyChange](title="TopologyChange in us"; source="topologyChange"; record=count,vector; interpolationmode=none);
        @statistic[fdbcleared](title="FDB cleared in us"; source="clearFDB"; record=count,vector; interpolationmode=none);
        @statistic[ringTime](title="T Ring in us"; source="receivedTest"; record=vector,stats,histogram; interpolationmode=none);
        @statistic[timeToChange](title="Time till announced Change in ms"; source="receivedChange"; record=vector,stats,histogram; interpolationmode=none);
        @statistic[sentContinuityCheck](title="Sent Continuity Checks"; source="continuityCheck"; record=count,vector; interpolationmode=none);
        @statistic[receivedContinuityCheck](title="Received Continuity Checks"; source="receivedContinuityCheck"; record=count,vector; interpolationmode=none);
        @statistic[portStateChanged](title="Time of Port State Change in us"; source="portStateChanged"; record=count,vector; interpolationmode=none);
        @statistic[ringStateChanged](title="Time of Ring State Change in us"; source="ringStateChanged"; record=count,vector; interpolationmode=none);
        @statistic[inTopologyChangeSent](title="InterconnectionTopologyChange sent"; source="inTopologyChange"; record=count,vector; interpolationmode=none);
        @statistic[inLinkChange](title="InterconnectionLinkChange sent"; source="inLinkChange"; record=count,vector; interpolationmode=none);
        @statistic[interconnectionRingTime](title="T InterconnectionRing in us"; source="receivedInTest"; record=count,vector; interpolationmode=none);
        @statistic[timeToInterconnectionChange](title="Time till announced InterconnectionChange"; source="receivedInChange"; record=vector,stats,histogram; interpolationmode=none);
        @statistic[inStatusPollSent](title="Interconnection Status Poll Sent in us"; source="inStatusPoll"; record=count,vector; interpolationmode=none);
        @statistic[receivedInStatusPoll](title="Interconnection Status Poll received in us"; source="receivedInStatusPoll"; record=count,vector; interpolationmode=none);
        @statistic[inTestFramesSent](title="Interconnection Test Frames Sent in us"; source="inTest"; record=count,vector; interpolationmode=none);
        @statistic[interconnectionStateChanged](title="Time of Interconnection State Change"; source="interconnectionStateChanged"; record=count,vector; interpolationmode=none);
}

