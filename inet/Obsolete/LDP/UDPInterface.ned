//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//




//
// UDPAppInterface module handles the UDP transport of discovery message in
// the initialization phase of LDP operations.
//
// Operations
//
// The module, after finishing the initialization, starts sending Hello
// messages to all routers in the subnet. The multicast address for these
// destinations is 224.0.0.0. Consequently, the module will receive messages
// from peers UDPAppInterface modules. These messages will then be forwarded
// to the LDPInterface module.
//
// Collaborations
//
// The modules collaborators are LDPInterface module and Network layer
// module. The collaborations are similar to that in any UDP applications
// generally.
//
// Usages
//
// Local address (LSR's loopback address) and UDP port number are required for
// the module to operate correctly.
//
simple UDPAppInterface
{
    parameters:
        volatile double local_port;
        string local_addr;
    gates:
        input from_udp_processing;
        input fromAppl;
        output to_udp_processing;
        output toAppl;
}



//
// FIXME: add documentation here
//
module UDPInterface
{
    parameters:
        string local_addr;
    gates:
        input from_ip;
        input from_appl;
        output to_ip;
        output to_appl;
    submodules:
        udpAppInterface: UDPAppInterface {
            parameters:
                local_addr = local_addr;
                local_port = 100;
                @display("p=91,50;b=40,24,rect");
        };
        udpProcessing: UDPProcessing {
            parameters:
                @display("p=94,105");
            gates:
                from_application[1];
                to_application[1];
        };
    connections allowunconnected:
        from_ip --> udpProcessing.from_ip;
        to_ip <-- udpProcessing.to_ip;
        udpAppInterface.fromAppl <-- from_appl;
        udpAppInterface.toAppl --> to_appl;

        udpProcessing.to_application[0] --> udpAppInterface.from_udp_processing;
        udpProcessing.from_application[0] <-- udpAppInterface.to_udp_processing;
}
