//-------------------------------------------------------------------------------
//      UnreliableChannel.ned --
//
//      This file defines "UnreliableChannel" module for RDT protocols.
//
//      Copyright (C) 2010 Kyeong Soo (Joseph) Kim
//-------------------------------------------------------------------------------

package inet.util;

// Models an unreliable channel where packets may experience bit errors
// (resulting in packet error) and packet losses.
simple UnreliableChannel
{
    parameters:
        double datarate @unit(bps) = default(100Mbps);
        double delay @unit(s) = default(0s);	// propagation delay in one direction (not round-trip delay)
        double per = default(0.0);	// packet error rate in [0, 1]
        double plr = default(0.0);	// packet loss rate in [0, 1]
        @display("i=misc/cloud3_vl");
    gates:
        inout leftGate;
        inout rightGate;
}
