//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Defines the interface for modules that implement the Media Redundancy Protocol (MRP),
// specified in IEC 62439-2. MRP provides high-availability in ring network topologies
// by ensuring a loop-free logical topology while maintaining redundancy.
//
// MRP operates by designating one node as the Media Redundancy Manager (MRM) that
// blocks one of its ports to prevent loops, while all other nodes act as Media
// Redundancy Clients (MRC). In case of a link or node failure, the MRM can quickly
// reconfigure the network by unblocking its port, ensuring continued connectivity
// with minimal disruption.
//
// This interface defines the essential parameters and gates required for MRP
// functionality, including:
// - Role configuration (MRM, MRC, or MRA)
// - Ring port assignments
// - Timing profiles for recovery
// - UUID for ring identification
// - Various operational flags and settings
//
// @see ~Mrp, ~MrpInterconnection, ~MrpRelay, ~MrpMacForwardingTable
//
moduleinterface IMrp
{
    parameters:
        @display("i=block/network2");
        double ccmInterval @unit(ms);
        string mrpRole;
        int timingProfile @unit(ms);
        int uuid0;
        int uuid1;
        int ringPort1;
        int ringPort2;
        bool interconnectionLinkCheckAware;
        bool interconnectionRingCheckAware;
        bool nonblockingMrcSupported;
        bool reactOnLinkChange;
        bool visualize;
    gates:
        input relayIn;
        output relayOut;
}
