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
// A generic bus model.
//
// in[i], out[i] gate pairs represent taps. Messages arriving on a tap
// travel on the bus on both directions, and copies of it are sent out
// on every other tap after delays proportional to their distances.
//
// Tap connections should have zero delays and zero data rates!
//
// Messages are not interpreted by the bus model in any way, thus the bus
// model is not specific to Ethernet in any way. Messages may
// represent anything, from the beginning of a frame transmission to
// end (or abortion) of transmission.
//
simple EtherBus
    parameters:
        positions : string,  // positions of taps on the cable in meters, as a sequence
                             // of numbers separated by spaces. If there are too
                             // few values, the distance between the last two positions
                             // is repeated, or 5 meters is used.
        propagationSpeed : numeric,
                             // signal propagation speed on the bus, in m/s
                             // (typically 200,000 m/s)
        writeScalars: bool;  // enable/disable recording statistics in omnetpp.sca
    gates:
        in: in[];    // to stations; each in[k], out[k] pair represents a tap
        out: out[];  // to stations; each in[k], out[k] pair represents a tap
endsimple

