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
// A generic wiring hub model.
//
// in[i], out[i] gate pairs represent ports. Messages arriving on a port
// are broadcast to every other port.
//
// The connections should have data rate set to zero. Cable lengths
// should be reflected in the delays of the connections.
//
// Messages are not interpreted by the hub model in any way, thus the hub
// model is not specific to Ethernet in any way. Messages may
// represent anything, from the beginning of a frame transmission to
// end (or abortion) of transmission.
//
simple EtherHub
    parameters:
        writeScalars: bool;   // enable/disable recording statistics in omnetpp.sca

    gates:
        in: in[];    // to stations; each in[k], out[k] pair represents a port
        out: out[];  // to stations; each in[k], out[k] pair represents a port
endsimple

