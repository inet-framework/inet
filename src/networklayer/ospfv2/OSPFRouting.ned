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


package inet.networklayer.ospfv2;

//
// Implements \OSPFRouting.
//
simple OSPFRouting
{
    parameters:
        string ospfConfigFile; // xml file containing the full OSPF AS configuration
        @display("i=block/network2");
    gates:
        input ipIn @labels(IPControlInfo/up);
        output ipOut @labels(IPControlInfo/down);
}
