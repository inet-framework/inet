//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

package inet.routing.lisp;

import inet.applications.contract.IApp;
import inet.common.SimpleModule;

//
// Locator/ID Separation Protocol (LISP, RFC 6830). A LISP node acts as some
// combination of ITR/ETR/PITR/PETR and optionally Map-Server/Map-Resolver. It
// exchanges control messages (Map-Request/Reply/Register/Notify) over UDP port
// 4342 and tunnels data over UDP port 4341, intercepting the data path with
// network-layer hooks. Slots into a host/router as an application (`like IApp`).
//
// NOTE: scaffold — message handling, mapping system and data plane are not yet
// implemented.
//
simple Lisp extends SimpleModule like IApp
{
    parameters:
        @class(Lisp);
        @display("i=block/network2");
        string interfaceTableModule = default("^.interfaceTable");
        string routingTableModule = default("^.ipv4.routingTable");
        string networkProtocolModule = default("^.ipv4.ip"); // for data-plane hooks (ITR/ETR)
        int controlPort = default(4342);   // LISP control messages
        int dataPort = default(4341);      // LISP encapsulated data
        string tunInterface = default("tun0"); // TUN interface used for the ITR/ETR data plane
        xml configData = default(xml("<LISP/>"));

        // roles (may be overridden by the <MapServer>/<MapResolver> config tags)
        bool mapServerV4 = default(false);
        bool mapServerV6 = default(false);
        bool mapResolverV4 = default(false);
        bool mapResolverV6 = default(false);
        bool hasSiteDB = default(false);

        // behaviour
        bool acceptMapRequestMapping = default(false); // cache the mapping carried in a Map-Request
        bool advertOnlyOwnEids = default(false);       // only register EIDs of this node's interfaces
        bool echoNonceAlgo = default(false);
        bool ciscoStartupDelays = default(false);
        int mapCacheTtl = default(1440);               // minutes
        string rlocProbingAlgo @enum("Cisco","Simple","Sophisticated") = default("Cisco");
    gates:
        input socketIn;
        output socketOut;
}
