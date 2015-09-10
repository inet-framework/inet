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

package inet.routing.contract;

//
// Interface for routing protocol modules connected to IP Layer.
//
moduleinterface IIPRouting
{
    gates:
        input ipIn @labels(IPv4ControlInfo/up);
        output ipOut @labels(IPv4ControlInfo/down);
}
