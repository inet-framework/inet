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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.linklayer.ethernet.switch;

import inet.linklayer.IMACRelayUnit;


//
// See: ~IMACRelayUnit
//
simple Ieee8021DRelay like IMACRelayUnit
{
    parameters:
        @display("i=block/broadcast;bgb=425,114");
        string interfaceTablePath = default("^.interfaceTable"); // the absolute path to the InterfaceTable
        string macTablePath = default("^.macTable"); // the absolute path to the MACAddressTable
    gates:
        input ifIn[];
        output ifOut[];
        input stpIn @loose;
        output stpOut @loose;
}
