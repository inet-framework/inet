//
// Copyright (C) 2003 CTIE, Monash University
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

import
    "EtherTrafGen",
    "Sink",
    "EtherMAC",
    "EtherLLC";


//
// Example host model with one Ethernet port and several traffic generators
// to create traffic in the test network.
//
module EtherHost
    parameters:
        numApps : numeric;  // number of "applications"

    gates:
        in: in;
        out: out;

    submodules:
        gen: EtherTrafGen[numApps];
            display: "p=60,60,col";
        sink: EtherTrafSink[numApps];
            display: "p=250,60,col";
        llc: EtherLLC;
            gatesizes:
                UpperLayer_in[numApps],
                UpperLayer_out[numApps];
            display: "i=fork;p=155,120";
        mac: EtherMAC;
            display: "i=queue;p=155,200";
    connections:
        llc.LowerLayer_in <-- mac.UpperLayer_out;
        llc.LowerLayer_out --> mac.UpperLayer_in;
        mac.LowerLayer_in <-- in;
        mac.LowerLayer_out --> out;

        for i=0..numApps-1 do
            gen[i].out --> llc.UpperLayer_in[i];
            sink[i].in <-- llc.UpperLayer_out[i];
        endfor;
endmodule

