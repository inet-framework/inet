//
// Copyright (C) 2010 Zoltan Bojthe
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
// Interface for Ethernet MAC implementations. All EtherMAC implementations should                                                                                                                                                                                             
// implement this (i.e. EtherMAC like IEtherMAC)                                                                                                                                                                                                                                    
//                                                                                                                                                                                                                                                                        
moduleinterface IEtherMAC
{
    parameters:
        bool promiscuous;       // if true, all packets are received, otherwise only the
                                // ones with matching destination MAC address
        string address;         // MAC address as hex string (12 hex digits), or
                                // "auto". "auto" values will be replaced by
                                // a generated MAC address in init stage 0.
        bool duplexEnabled;     // whether duplex mode can be enabled or not; whether
                                // MAC will actually use duplex mode depends on the result
                                // of the auto-configuration process (duplex is only
                                // possible with DTE-to-DTE connection).
        int txQueueLimit;       // maximum number of frames queued up for transmission;
                                // additional frames are dropped. Only used if queueModule==""
        string queueModule;     // name of optional external queue module
        bool frameBursting;     // enable/disable frame bursting mode in Gigabit Ethernet
        int mtu;
        @display("i=block/rxtx");
    gates:
        input upperLayerIn @labels(EtherFrame);    // to EtherLLC or MACRelayUnitPP
        output upperLayerOut @labels(EtherFrame);  // to EtherLLC or MACRelayUnitPP
        inout phys @labels(EtherFrame); // to physical layer or the network
}
