//
// Copyright (C) 2005 Vojtech Janota, Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ted;

import inet.common.SimpleModule;

//
// Implements a very minimalistic link state routing protocol.
// Apart from the basic topology information, the current link usage
// is distributed to all participants in the network (by means of flooding).
//
// Collaborations
//
// The link state database is kept in the ~Ted module.
//
// This is a topology mirror, not a general-purpose routing protocol: it
// floods the local ~Ted contents to a fixed set of peers configured via the
// "peers" parameter (no periodic re-origination, no LSA aging, no wraparound
// handling for message ids). Like ~Ted, ~Ldp, and ~RsvpTe, it extends the
// lifecycle-aware routing protocol base class: a node shutdown cancels its
// pending announce and clears the peer snapshot, and a restart recomputes
// the peer snapshot from the (by-then possibly changed) ~Ted contents and
// restarts flooding -- so a restarted node's flooding activity reflects its
// current topology rather than a stale snapshot taken at simulation start.
//
simple LinkStateRouting extends SimpleModule
{
    parameters:
        @class(LinkStateRouting);
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string tedModule;
        string peers;
        @display("i=block/network2");
    gates:
        input ipIn @labels(Ipv4ControlInfo/up);
        output ipOut @labels(Ipv4ControlInfo/down);
}

