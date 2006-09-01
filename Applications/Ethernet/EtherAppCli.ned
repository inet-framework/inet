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
// Simple traffic generator for the Ethernet model and the 802.11 model;
// generally any L2 model that accepts Ieee80211Ctrl control info
// on the packets. It generates EtherAppReq packets. It should be connected
// directly to EtherLLC or a Ieee802NicXXX module.
//
// Destination may be given with OMNeT++ path name or MAC address.
//
simple EtherAppCli
    parameters:
        destStation: string,  // module path name of destination station
                              // (must be blank if destAddress is given)
        destAddress: string,  // destination MAC address
                              // (must be blank if destStation is given)
        waitTime: numeric,    // interarrival time between sending requests; may be random value
        reqLength: numeric,   // length of request packets (bytes); may be random value
        respLength: numeric,  // length of response packets (bytes); may be random value
        registerSAP : bool,   // whether to sent IEEE802CTRL_REGISTER_DSAP on startup
        writeScalars: bool;   // enable/disable recording statistics in omnetpp.sca

    gates:
        in: in;
        out: out;
endsimple
