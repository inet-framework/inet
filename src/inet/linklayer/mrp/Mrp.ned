//
// Copyright (C) 2024 Daniel Zeitler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//



package inet.linklayer.mrp;

import inet.linklayer.contract.IMediaRedundancy;


//
// Implements the Media Redundancy Protocol (MRP) as specified in IEC 62439-2.
// MRP is used for managing redundancy in ring network topologies commonly found
// in industrial network settings. MRP is similar in its function to STP and
// RSTP, in that it ensures a loop-free topology for an Ethernet local area
// network. This module implements the Media Redundancy Manager (MRM), Media
// Redundancy Client (MRC), and Media Redundancy Automanager (MRA) roles. This
// model also includes support for detecting link errors with the help of
// Continuity Check Messages (CCM) messages defined in 802.1q CFM.
//
// Note: Support for interconnecting rings, i.e. for the Media Redundancy
// Interconnection Manager (MIM) and Media Redundancy Interconnection Client
// (MIC) roles, is implemented in the ~MrpInterconnection module.
//
// The basic function of MRP is to prevent bridge loops by establishing a
// logical line topology based on a physical ring topology. MRP nodes must be
// directly connected to each other, with a maximum of 50 nodes per ring as
// defined in the standard. In the event of a network, link, or node error, a
// node in the manager role can alter the topology by activating its secondary
// ring port to reestablish a functioning network within a guaranteed maximum
// time limit.
//
// An MRP node can be either a switch or a single device. Each node has exactly
// two ports that are part of the MRP ring, while all other ports function as
// normal switch ports.
//
// The health of the ring is monitored by periodic test frames sent by the node
// in the manager role. If a test frame is not received on the opposite ring
// port of the node within five test intervals, a topology change is initiated.
// Ring failures may also be detected by clients, which can locally identify
// link errors with the help of 802.1q CCM messages. In such cases, a client
// sends a link change frame, prompting the manager to potentially initiate a
// topology change.
//
// MRP makes use of per-port configuration data defining role and state. They
// can be set using ~L2NetworkConfigurator. Port state is kept in the switch's
// ~InterfaceTable.
//
// To configure MRP in a network, follow these steps:
//
// 1. Identify and Configure Rings: Assign a unique ring ID to all nodes within the
//    same ring using the `uuid0` and `uuid1` parameters. This ensures that each node
//    is correctly recognized as part of the ring topology.
//
// 2. Assign Ring Ports: Configure the `ringPort1` and `ringPort2` parameters.
//    If you already set up the network topology so the ring ports are index 0
//    and 1, these parameters do not need to be touched as the default values will suffice.
//
// 3. Set Node Roles: Use the `mrpRole` parameter to assign roles within the ring.
//    Designate one node as the Media Redundancy Manager (MRM) and the others as
//    Media Redundancy Clients (MRC). Alternatively, you can set all nodes to Media
//    Redundancy Automanager (MRA).
//
// 4. Adjust Features: Modify optional feature flags and settings as needed.
//
// @see ~MrpInterconnection, ~MrpRelay, ~MrpMacForwardingTable, ~L2NetworkConfigurator, ~InterfaceTable
//
simple Mrp like IMediaRedundancy
{
    parameters:
        @class(Mrp);

        // Switches do not detect a link change immediately. This parameter sets
        // the simulated delay of a "link down" event. Note: the delay for "link
        // up" is hardcoded to 1us in the code.
        volatile double linkDetectionDelay @unit(s) = default(truncnormal(385ms,325ms));

        // Switches require a certain amount of time to process each received packet.
        // This parameter sets the processing delay; it is applied in handleMessageWhenUp().
        volatile double processingDelay @unit(s) = default(truncnormal(100us,80us));

        // MRP defines four Maximum Recovery Time settings for rings: 500 ms, 200 ms, 30 ms, or 10 ms.
        // This parameter sets a hard upper time limit guaranteed by the protocol,
        // and the interval for test frames, topology changes, and link change times are adapted accordingly.
        // It should be set to the same value on all nodes within a ring.
        int timingProfile @unit(ms) = default(500ms);

        // Each MRP node can either be disabled (value 0, no MRP function), be a MediaRedundancyClient (MRC, value 1),
        // a MediaRedundancyManager (MRM, value 2), or a MediaRedundancyAutoManager (MRA, value 4).
        // The model does not support the backward-compatible MRA (value 3) as it would be redundant.
        // Each network may have only one MRM; all other nodes must assume the MRC role.
        // MRAs can vote on which node should take the MRM role based on priority and MAC address value; other nodes
        // dynamically switch to the MRC role. Mixing MRM and MRA is possible but not recommended.
        string mrpRole @enum("disabled", "MRC", "MRM", "MRA") = default("disabled");

        // Each MRP node has exactly two ring interfaces, which must be configured by
        // the network administrator. The ringPort1 and ringPort2 parameters define
        // the port index of the first and second ring port, respectively.
        int ringPort1 = default(0);
        int ringPort2 = default(1);

        // An MRP ring has a UUID, split into two 64-bit parts.
        // Every node in a ring must be configured with the same UUID.
        int uuid0;
        int uuid1;

        // It is possible to enable active link checks on Layer 2 for each ring port,
        // using CCM (Connectivity Check Message) messages defined in 802.1q Continuity
        // Fault Management (CFM). Consider the additional load on the node and network
        // when enabling this feature.
        bool enableLinkCheckOnRing = default(false);

        // CCM (Connectivity Check Message) link checks can be performed every 3.3 ms or every 10 ms.
        double ccmInterval @unit(s) = default(10ms);

        // MRP rings can be redundantly connected by interconnection nodes.
        // To manage necessary communication, normal MRP nodes should be aware of the
        // check methods used. Checks can be performed either by ring check (similar to the
        // ring test mechanism) or by link check on Layer 2 (CCM messages).
        bool interconnectionRingCheckAware = default(true);
        bool interconnectionLinkCheckAware = default(true);

        // Visualizes the ring by coloring connections in the network graphics.
        // Labels the Ethernet interface with port role and status.
        bool visualize = default(true);

        // The path to the InterfaceTable module.
        string interfaceTableModule;

        // The path to the MacForwardingTable module.
        string macTableModule;

        // The path to the MrpRelay module.
        string mrpRelayModule;

        // A MRM or MRA can have a priority ranging from the highest (0x0000) to the lowest (0xFFFF).
        // MRMs should be assigned one of the following values: HIGHEST = 0x0000, HIGH = 0x4000, DEFAULT = 0x8000.
        // If the node is an MRA, assign one of these: MRAHIGHEST = 0x9000, MRADEFAULT = 0xA000, MRALOWEST = 0xFFFF.
        // MRAs determine the acting manager based on priority and MAC address.
        int mrpPriority = default(0xA000);

        // In MRM role: Specifies whether non-blocking MRCs are supported by the MRM,
        // i.e. those that are not capable of setting a ring port to BLOCKED state.
        bool nonblockingMrcSupported = default(true);

        // In MRM role: Determines if the manager reacts to link change frames sent
        // by clients or if it solely relies on ring tests for status updates.
        bool reactOnLinkChange = default(true);

        // Format for the text displayed above the module icon.
        // Directives: `%r`: MRP role, `%n`: node state, `%g`: ring state
        string displayStringTextFormat = default("role: %r\nnode: %n\nring: %g");

        @display("i=block/network2");
        @signal[ringStateChanged](type=long);
        @signal[nodeStateChanged](type=long);
        @signal[ringPort1StateChanged](type=unsigned long);
        @signal[ringPort2StateChanged](type=unsigned long);
        @signal[topologyChangeAnnounced](type=long); // emitted by MRM only with value = constant 1
        @signal[fdbCleared](type=long);  // value = constant 1
        @signal[linkChangeDetected](type=long);  // emitted by the node that detected the link change; value = constant 0 or 1 (for down/up)
        @signal[testFrameLatency](type=simtime_t); // emitted by the MRM only, upon receiving a Test frame --TODO rename to ringTime?
        @statistic[ringState](title="Ring state"; source=ringStateChanged; record=vector; interpolationmode=sample-hold; enumname=inet::Mrp::RingState);
        @statistic[nodeState](title="Node state"; source=nodeStateChanged; record=vector; interpolationmode=sample-hold; enumname=inet::Mrp::NodeState);
        @statistic[ringPort1State](title="State of Primary Ring Port"; source=ringPort1StateChanged; record=vector; interpolationmode=sample-hold; enumname=inet::MrpInterfaceData::PortState);
        @statistic[ringPort2State](title="State of Secondary Ring Port"; source=ringPort2StateChanged; record=vector; interpolationmode=sample-hold; enumname=inet::MrpInterfaceData::PortState);
        @statistic[topologyChangeAnnouncements](title="Topology Change Announcements by MRM"; source=topologyChangeAnnounced; record=vector; interpolationmode=none);
        @statistic[fdbClearedEvents](title="FDB Cleared Events"; source=fdbCleared; record=vector; interpolationmode=none);
        @statistic[linkChangeDetections](title="Link Up/Down Detection Events"; source=linkChangeDetected; record=vector; interpolationmode=none);
        @statistic[testFrameLatency](title="Latency of Received Test Frames"; source=testFrameLatency; record=vector; interpolationmode=none);

    gates:
        input relayIn;
        output relayOut;
}
