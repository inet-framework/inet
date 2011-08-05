//
// Copyright (C) 2006 Autonomic Networking Group, 
// Department of Computer Science 7, University of Erlangen, Germany
//
// Author: Isabel Dietrich
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

package inet.applications.trafgen;

simple TrafGenPar
{
        bool isSink = default(true); // if true the node doesn't send traffic
        volatile int packetSize @unit("B")= default(10B);
	    volatile double interDepartureTime @unit("s") = default(exponential(1s));
	    volatile double firstPacketTime @unit("s")= default(5s);
	    string   trafDest =  default("BROADCAST");
	    bool      onIdenticalTrafDest = default (false);
	    volatile double onLength @unit("s")= default(0s);
	    volatile double offLength @unit("s")= default(0s);
	    volatile double offInterDepartureTime @unit("s")= default(0s);
    //    xml trafConfig;
    gates:
        input lowerLayerIn;
        output lowerLayerOut;
}

