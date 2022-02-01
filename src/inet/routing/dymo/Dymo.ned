//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.routing.dymo;

import inet.routing.contract.IManetRouting;

//
// This module provides Dynamic MANET On-demand (Dymo also known as AODVv2) Routing
// based on the IETF draft at http://tools.ietf.org/html/draft-ietf-manet-dymo-24.
//
// Module parameters are copied from the IETF draft and renamed for consistency with
// INET naming conventions. Some parameters are not yet implemented.
//
simple Dymo like IManetRouting
{
    parameters:
        // properties
        @class("dymo::Dymo");
        @display("i=block/routing");

        // context parameters
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule = default(absPath("^.ipv4.routingTable"));
        string networkProtocolModule = default(absPath("^.ipv4.ip"));

        // 1. Dymo parameter group
        string clientAddresses = default("10.10.0.0/24 10.20.0.0"); // CLIENT_ADDRESSES
        bool useMulticastRREP = default(false); // USE_MULTICAST_RREP
        // string DEFAULT_METRIC_TYPE = default("HOP_COUNT");
        string interfaces = default("*"); // AODVv2_INTERFACES

        // 2. Dymo parameter group
        double activeInterval @unit(s) = default(5s); // ACTIVE_INTERVAL
        double maxIdleTime @unit(s) = default(200s); // MAX_IDLETIME
        double maxSequenceNumberLifetime @unit(s) = default(300s); // MAX_SEQNUM_LIFETIME
        double routeRREQWaitTime @unit(s) = default(2s); // ROUTE_RREQ_WAIT_TIME
        // double UNICAST_MESSAGE_SENT_TIMEOUT @unit(s) = default(1s);
        double rreqHolddownTime @unit(s) = default(10s); // RREQ_HOLDDOWN_TIME

        // 3. Dymo parameter group
        int maxHopCount = default(20); // MAX_HOPCOUNT
        // string MAX_METRIC = default("");
        // double MAXTIME;
        int discoveryAttemptsMax = default(3); // DISCOVERY_ATTEMPTS_MAX
        // int MTU;

        // 4. Dymo parameter group
        bool appendInformation = default(true); //APPEND_INFORMATION;
        int bufferSizePackets = default(-1); // BUFFER_SIZE_PACKETS
        int bufferSizeBytes @unit(B) = default(-1B); //  BUFFER_SIZE_BYTES
        // double CONTROL_TRAFFIC_LIMIT

        // Dymo extension parameters
        double maxJitter @unit(s) = default(10ms); // see IETF RFC5148 (Jitter Considerations in MANET)
        bool sendIntermediateRREP = default(true);
        int minHopLimit = default(5);
        int maxHopLimit = default(10);

    gates:
        input ipIn;
        output ipOut;
}

