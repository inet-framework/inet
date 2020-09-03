//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

package inet.linklayer.vlan;

import inet.linklayer.contract.IVlanInterface;
import inet.networklayer.common.NetworkInterface;

//
// This module implements a virtual network interface.
//
module VlanInterface extends NetworkInterface like IVlanInterface
{
    parameters:
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        @display("i=block/tunnel");
    gates:
        input upperLayerIn;
        output upperLayerOut;
    submodules:
        tunnel: VlanTunnel {
            parameters:
	            @display("p=300,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> tunnel.upperLayerIn;
        tunnel.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
}

