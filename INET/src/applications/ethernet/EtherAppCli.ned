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

package org.omnetpp.inet.Applications.Ethernet;

//
// Simple traffic generator for the Ethernet model and the 802.11 model;
// generally any L2 model that accepts Ieee80211Ctrl control info
// on the packets. It generates EtherAppReq packets. It should be connected
// directly to EtherLLC or a Ieee802NicXXX module.
//
// Destination may be given with OMNeT++ path name or MAC address.
//
simple EtherAppCli
{
    parameters:
        string destStation;  // module path name of destination station
                              // (must be blank if destAddress is given)
        string destAddress = default("");  // destination MAC address
                              // (must be blank if destStation is given)
        volatile double waitTime @unit("s");    // interarrival time between sending requests; may be random value
        volatile int reqLength;   // length of request packets (bytes); may be random value
        volatile int respLength;  // length of response packets (bytes); may be random value
        bool registerSAP;   // whether to sent IEEE802CTRL_REGISTER_DSAP on startup
        bool writeScalars = default(true);   // enable/disable recording statistics in omnetpp.sca

    gates:
        input in;
        output out;
}
