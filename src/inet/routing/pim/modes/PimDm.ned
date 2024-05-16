//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Authors: Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)

package inet.routing.pim.modes;

//
// Implementation of PIM-DM protocol (RFC 3973).
//
// PIM-DM is a multicast routing protocol which is designed
// for networks where the receivers are densely distributed.
// PIM-DM initially floods the multicast datagrams to all areas
// of the network. If some areas of the network do not have
// group members, then they will be pruned off the forwarding tree.
//
// The prune state of a branch must be confirmed periodically,
// otherwise the forwarding restarts automatically. The router
// can also cancel the prune by sending a Graft message.
//
// The module must access the ~PimInterfaceTable and ~PimNeighborTable
// modules, their path is given as the `pimInterfaceTableModule` and
// `pimNeighborTableModule` parameters.
//
// Other parameters are timer values from the specification.
// They should not be modified except for testing purposes.
//
simple PimDm
{
    parameters:
        @display("i=block/network2");
        string interfaceTableModule;
        string routingTableModule;
        string pimInterfaceTableModule;
        string pimNeighborTableModule;

        volatile double triggeredHelloDelay @unit(s) = uniform(0s,5s);
        double helloPeriod @unit(s) = 30s; // "Hello_Period: Periodic interval for hello messages"
        double holdTime @unit(s) = default(3.5 * helloPeriod); // "The Hold Time in the Hello Message should be set to a value that can reasonably be expected to keep the Hello active until a new Hello message is received.  On most links, this will be 3.5 times the value of Hello_Period."
        double pruneInterval @unit(s) = 180s; // "The Prune Timer expires, indicating that it is again time to flood data from S addressed to group G onto interface I."
        double pruneLimitInterval @unit(s) = 210s; // "t_limit: Used to prevent Prune storms on a LAN"
        double overrideInterval @unit(s) = 2.5s; // "J/P_Override_Interval: Short time after a Prune to allow other routers on the LAN to send a Join"
        double propagationDelay @unit(s) = 0.5s;
        double graftRetryInterval @unit(s) = 3s; // "Graft_Retry_Period: In the absence of receipt of a GraftAck message, the time before retransmission of a Graft message"
        double sourceActiveInterval @unit(s) = 210s; // "SourceLifetime: Period of time after receiving a multicast message a directly attached router will continue to send State Refresh messages"
        double stateRefreshInterval @unit(s) = 60s; // "RefreshInterval: Interval between successive state refresh messages"
        double assertTime @unit(s) = 180s; // "Assert Time: Period after last assert before assert state is timed out"

        @signal[sentHelloPk](type=cPacket);
        @signal[rcvdHelloPk](type=cPacket);
        @signal[sentGraftPk](type=cPacket);
        @signal[rcvdGraftPk](type=cPacket);
        @signal[sentGraftAckPk](type=cPacket);
        @signal[rcvdGraftAckPk](type=cPacket);
        @signal[sentJoinPrunePk](type=cPacket);
        @signal[rcvdJoinPrunePk](type=cPacket);
        @signal[sentAssertPk](type=cPacket);
        @signal[rcvdAssertPk](type=cPacket);
        @signal[sentStateRefreshPk](type=cPacket);
        @signal[rcvdStateRefreshPk](type=cPacket);

    gates:
        input ipIn;
        output ipOut;
}

