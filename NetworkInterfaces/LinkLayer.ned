//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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
    "OutputQueue",
    "NetworkInterface";


//
// Represents a network interface.
//
module LinkLayer
    parameters:
        NWIName : string;
    gates:
        in: physIn;
        in: nwIn;
        out: physOut;
        out: nwOut; 
                    
    submodules:
        outputQueue: OutputQueue;
            display: "b=32,30;p=88,67,row;i=queue";
        networkInterface: NWIName like NetworkInterface;
            display: "b=32,30;p=88,153;i=prot1";
    connections nocheck:
        nwIn --> outputQueue.fromIP;
        outputQueue.toNW --> networkInterface.ipOutputQueueIn;
        outputQueue.fromNW <-- networkInterface.ipOutputQueueOut;

        networkInterface.physicalOut --> physOut;

        networkInterface.physicalIn <-- physIn;
        networkInterface.ipOutputQueueOut --> nwOut display "m=e";
endmodule

