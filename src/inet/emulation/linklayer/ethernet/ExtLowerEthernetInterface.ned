//
// Copyright (C) 2004 Andras Varga
//               2010 Zoltan Bojthe
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

package inet.emulation.linklayer.ethernet;

import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.contract.IExternalInterface;
import inet.linklayer.ethernet.IEtherEncap;

module ExtLowerEthernetInterface like IExternalInterface, IEthernetInterface
{
    parameters:
        string interfaceTableModule;
        int mtu @unit(B) = default(4470B);
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        @class(ExtInterface);
        @display("i=block/ifcard");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys @loose;
    submodules:
        encap: <default("EtherEncap")> like IEtherEncap {
            parameters:
                @display("p=100,100");
        }
        socket: ExtEthernetDeviceSocket {
            parameters:
                @display("p=100,300");
        }
    connections allowunconnected:
        upperLayerIn --> { @display("m=n"); } --> encap.upperLayerIn if exists(encap);
        upperLayerOut <-- { @display("m=n"); } <-- encap.upperLayerOut if exists(encap);
        encap.lowerLayerOut --> socket.upperLayerIn if exists(encap);
        encap.lowerLayerIn <-- socket.upperLayerOut if exists(encap);

        upperLayerIn --> { @display("m=n"); } --> socket.upperLayerIn if !exists(encap);
        upperLayerOut <-- { @display("m=n"); } <-- socket.upperLayerOut if !exists(encap);
}
