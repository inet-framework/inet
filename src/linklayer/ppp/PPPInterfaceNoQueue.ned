//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//



package inet.linklayer.ppp;

import inet.linklayer.INetworkInterface;


//
// \PPP interface. Assumes inifinite queue.
//
// @see PPPInterface
//
module PPPInterfaceNoQueue like INetworkInterface
{
    parameters:
        @display("i=block/ifcard");
    gates:
        input netwIn;
        output netwOut;
        inout phys @labels(PPPFrame);
    submodules:
        ppp: PPP {
            parameters:
                queueModule = "";
                txQueueLimit = 100000; // prevents overflow in bogus simulations
                @display("p=80,80;q=txQueue");
        }
    connections:
        netwIn --> { @display("m=n"); } --> ppp.netwIn;
        netwOut <-- { @display("m=n"); } <-- ppp.netwOut;
        phys <--> { @display("m=s"); } <--> ppp.phys;
}


