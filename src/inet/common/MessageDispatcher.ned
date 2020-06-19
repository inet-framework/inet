//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.common;

import inet.queueing.base.PacketProcessorBase;

//
// This module connects multiple applications, protocols and interfaces with
// each other and automatically dispatches messages and packets between them.
// It allows many different configurations from layered architectures where
// message dispatchers separate different communication layers to centralized
// architectures where a single message dispatcher is connected to all components.
//
// Configuring the dispatch mechanism:
//  - protocols must register by calling registerService and/or registerProtocol
//  - interfaces must register by calling registerInterface
//  - sockets must register by sending socket messages
//  - packets must have tags attached which specifie the destination protocol, interface or socket
//
simple MessageDispatcher extends PacketProcessorBase
{
    parameters:
        @class(MessageDispatcher);
        @display("i=-;b=100,5,,,,1;");
    gates:
        input in[];
        output out[];
}

