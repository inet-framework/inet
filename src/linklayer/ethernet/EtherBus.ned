//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
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


package inet.linklayer.ethernet;

//
// Implements the shared coaxial cable for classic Ethernet.
//
// The ethg[i] gates represent taps. Messages arriving on a tap
// travel on the bus on both directions, and copies of it are sent out
// on every other tap after delays proportional to their distances.
//
// For the model to work correctly, all connecting links (both incoming 
// and outgoing ones) must have the same datarate.
//
// Messages are not interpreted by the bus model in any way, thus the bus
// model is not specific to Ethernet.
//
// It is allowed to disconnect/reconnect links at runtime. However, 
// the model does not support changing the tap positions or adding/removing
// taps at runtime.
//
simple EtherBus
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        @display("b=200,5,,,,1");
        string positions;  // positions of taps on the cable in meters, as a sequence
                           // of numbers separated by spaces. If there are too
                           // few values, the distance between the last two positions
                           // is repeated, or 5 meters is used.
        double propagationSpeed @unit("mps") = default(200000000mps); // signal propagation speed on the bus
    gates:
        inout ethg[] @labels(EtherFrame-conn);  // to stations; each one represents a tap
}

