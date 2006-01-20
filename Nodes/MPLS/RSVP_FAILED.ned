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

import
    "Dummy";

//
// Failed router. Discards all incoming traffic and generates nothing. Its
// interface (gates, parameters) is fully compatible with the RSVP_LSR router
// and node failures may thus be simulated by replacing the RSVP_LSR
// with \FAILED_LSR.
//
module RSVP_FAILED
    parameters:
        namid : numeric,
    
        peers : string,
        routerId : string,
        routingFile : string;
    gates:
        in: in[];
        out: out[];

    submodules:
        dummy: Dummy;
            gatesizes:
                in[sizeof(in)],
                out[sizeof(out)];

    connections nocheck:
        for i=0..sizeof(out)-1 do
            in[i] --> dummy.in[i];
                dummy.out[i] --> out[i];
        endfor;
endmodule
