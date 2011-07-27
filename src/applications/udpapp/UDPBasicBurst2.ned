//
// Copyright (C) 2006 Alfonso Ariza
// Copyright (C) 2004 Andras Varga
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


//
// Sends \UDP packets to the given \IP address at the given interval.
// Compatible with both IPv4 and IPv6.
//
// The interval can be a constant or a random value (e.g. exponential(1)).
// If the dest_addresses parameter contains more than one address, one
// of them is randomly chosen for each packet. An address may be given in the
// dotted decimal notation, or with the module name. (The IPAddressResolver
// class is used to resolve the address.) To disable the model, set
// dest_addresses to "".
//
// The peer can be UDPSink or another UDPBasicApp (it handles received packets
// exactly like UDPSink).
//
//#FIXME change message length to BYTES
//

package inet.applications.udpapp;


import inet.applications.IUDPApp;


simple UDPBasicBurst2 like IUDPApp
{
    parameters:
        int localPort;
        int destPort;
        volatile int messageLength @unit("B"); //  length of messages to generate, in bytes
        volatile double messageFreq @unit("s"); // should usually be a random value, e.g. exponential(1), if -1 the application is a sink
        volatile double message_freq_jitter @unit("s"); // should usually be a random value, e.g. uniform(-0.001,0.001)
        string destAddresses; // list of \IP addresses, separated by spaces, it's admit the macros radom("node type") and random_name("node name")
        volatile double time_off @unit("s");  // time without transmision
        volatile double burstDuration @unit("s");  // burst duration time
        bool activeBurst; // if false the address is selected in the begining and never change
        double time_end @unit("s"); // time of the last transmision packet 
        double time_begin @unit("s");// begin transmission time
        int rand_generator;// ID random generator
        double limitDelay @unit("s"); // maximun delay for a packet, every packet with bigger delay is delete and lost
        string FixNodeName = default("fixhost");
        bool fixedDestination = default(false);  // if true the destination address is selected at the begining and never change
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}
