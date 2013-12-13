//
// Copyright (C) 2011 Juan Luis Garrote Molinero
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

package inet.nodes.rstp;
import inet.linklayer.ethernet.EtherMAC;
import inet.linklayer.rstp.tester.TesterObject;

//
//	Tester node.
//
module Tester
{
    parameters:
        @node();
        @display("i=device/port;bgb=643,442");
		
    gates:
        inout Connection;  
       
    submodules:
        testerObject: TesterObject {
            parameters:
                @display("p=171,148");
            gates:
                Out;
                In;
        }
        mac: EtherMAC {
            parameters:
                promiscuous = true;
                queueModule = "";
                @display("p=171,53,row;q=queue");
        }

    connections:
		mac.upperLayerIn <-- testerObject.Out;
		mac.upperLayerOut --> testerObject.In;
		mac.phys <-->Connection;
}
