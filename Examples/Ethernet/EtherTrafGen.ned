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

//
// Simple traffic generator for the Ethernet model.
// Destination may be given with OMNeT++ path name or MAC address.
//
simple EtherTrafGen
    parameters:
        destStation: string,  // module path name of destination station
                              // (must be blank if destAddress is given)
        destAddress: string,  // destination MAC address
                              // (must be blank if destStation is given)
        protocolId: numeric,  // value for SSAP/DSAP in Ethernet frame
        waitMean: numeric,    // mean for exponential interarrival times
        dataLength: numeric;  // length of data packets (bytes); may be random value

    gates:
        out: out;
endsimple
