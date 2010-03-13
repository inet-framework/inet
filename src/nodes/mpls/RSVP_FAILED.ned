//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


package inet.nodes.mpls;

import inet.networklayer.extras.Dummy;


//
// Failed router. Discards all incoming traffic and generates nothing. Its
// interface (gates, parameters) is fully compatible with the RSVP_LSR router
// and node failures may thus be simulated by replacing the RSVP_LSR
// with \RSVP_FAILED.
//
module RSVP_FAILED
{
    parameters:
        @labels(node);
        @display("i=abstract/router;i2=status/noentry");
        int namid = default(-1);
        string peers;
        string routerId = default("auto");
        string routingFile = default("");
    gates:
        inout pppg[] @labels(PPPFrame-conn);
    submodules:
        dummy: Dummy {
            gates:
                in[sizeof(pppg)];
                out[sizeof(pppg)];
        }
    connections allowunconnected:
        for i=0..sizeof(pppg)-1 {
            pppg$i[i] --> dummy.in[i];
            dummy.out[i] --> pppg$o[i];
        }
}
