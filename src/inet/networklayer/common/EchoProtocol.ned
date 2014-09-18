//
// Copyright (C) 2004, 2009 Andras Varga
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

package inet.networklayer.common;

//
// This module provides a mechanism to test network layer connectivity
// using echo request/response messages similar to that of ICMP.
//
simple EchoProtocol
{
    parameters:
        @display("i=block/control_s");
    gates:
        input localIn @labels(GenericNetworkProtocolControlInfo/up);
        input pingIn[];  // ping requests from app
        output pingOut[];  // result of ping
        output sendOut @labels(GenericNetworkProtocolControlInfo/down);  // towards network
}
