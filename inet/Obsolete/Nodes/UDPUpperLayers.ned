//
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



module UDPUpperLayers
{
    parameters:
        string udpClient1Name;
        string udpServer1Name;
        @display("p=10,10;b=157,140,rect");
    gates:
        input from_ip;
        output to_ip;
    submodules:
        udpProcessing: UDPProcessing {
            parameters:
                @display("p=94,105");
            gates:
                from_application[2];
                to_application[2];
        };
        udpClient1: <udpClient1Name> like UDPApp {
            parameters:
                @display("p=134,43;b=48,32,rect");
        };
        udpServer1: <udpServer1Name> like UDPApp {
            parameters:
                @display("p=51,42;b=40,24,rect");
        };
    connections allowunconnected:
        from_ip --> udpProcessing.from_ip;
        to_ip <-- udpProcessing.to_ip;

        udpProcessing.to_application[0] --> udpClient1.from_udp;
        udpProcessing.from_application[0] <-- udpClient1.to_udp;

        udpProcessing.to_application[1] --> udpServer1.from_udp;
        udpProcessing.from_application[1] <-- udpServer1.to_udp;
}

