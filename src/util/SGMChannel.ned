//-------------------------------------------------------------------------------
//      SGMChannel.ned --
//
//      This file defines "SGMChannel" module.
//
//      Copyright (C) 2015 Kyeong Soo (Joseph) Kim
//-------------------------------------------------------------------------------

package inet.util;

// Simple Gilber model (SGM) for lossy channel.
simple SGMChannel
{
    parameters:
        double datarate @unit(bps) = default(100Mbps);
        double delay @unit(s) = default(0s);	// propagation delay in one direction (not round-trip delay)
        double p = default(0.1);	// transition probability from GOOD to BAD state
        double q = default(0.1);	// transition probability from BAD to GOOD state
        @display("i=misc/cloud3_vl");
    gates:
        inout leftGate;
        inout rightGate;
}
