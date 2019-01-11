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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

package inet.linklayer.common;

//
// This module classifies packets and assigns a User Priority based on the IP
// protocol or the transport protocol port numbers.
//
simple QosClassifier like IIeee8021dQosClassifier
{
    parameters:
        string ipProtocolUpMap = default("1 BE 58 BE");
        string udpPortUpMap @enum() = default("21 BK 80 BE 4000 VI 5000 VO");
        string tcpPortUpMap @enum() = default("21 BK 80 BE 4000 VI 5000 VO");
        string defaultUp = default("BE");
        @display("i=block/classifier");
    gates:
        input in;
        output out;
}

