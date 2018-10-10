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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.linklayer.vlan;

import inet.linklayer.contract.INetworkInterface;

moduleinterface IVlanInterface extends INetworkInterface
{
    parameters:
        @display("i=block/tunnel");
}

module VlanInterface like IVlanInterface
{
    parameters:
        string interfaceTableModule;
        string address = default("auto");
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        @display("i=block/tunnel");
        @class(VlanInterface);
    gates:
        input upperLayerIn;
        output upperLayerOut;
    submodules:
        vlan: Vlan {
            @display("p=100,100");
        }
    connections:
        upperLayerIn --> vlan.upperLayerIn;
        upperLayerOut <-- vlan.upperLayerOut;
}

simple Vlan
{
    parameters:
        string interfaceTableModule;
        int vlanId;
        string interfaceName;
        @display("i=block/cogwheel");
    gates:
        input upperLayerIn;
        output upperLayerOut;
}
