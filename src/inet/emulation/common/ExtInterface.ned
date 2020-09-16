//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.emulation.common;

import inet.linklayer.contract.IExternalInterface;
import inet.networklayer.common.NetworkInterface;

//
// This module provides a base module for external network interfaces.
//
module ExtInterface extends NetworkInterface like IExternalInterface
{
    parameters:
        string interfaceTableModule;
        string device;
        string namespace = default("");
        string copyConfiguration @enum("","copyToExt","copyFromExt") = default("");
        string protocol;
        string address = default("auto");
        bool broadcast = default(true);
        bool multicast = default(true);
        bool pointToPoint = default(false);
        int mtu @unit(B) = default(4470B);
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.device = default(device);
        *.namespace = default(namespace);
        @class(ExtInterface);
    gates:
        input upperLayerIn @loose;
        output upperLayerOut @loose;
}

