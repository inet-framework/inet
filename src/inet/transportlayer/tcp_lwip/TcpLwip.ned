//
// Copyright (C) 2010 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

package inet.transportlayer.tcp_lwip;

import inet.transportlayer.contract.ITcp;

//
// TCP model based on the LWIP 1.3.2 project.
// The original LWIP is available on
// <a href="http://savannah.nongnu.org/projects/lwip/" target="_blank">this page</a>.
//
// See ~ITcp for general TCP layer informations.
//
simple TcpLwip like ITcp
{
    parameters:
        string sendQueueClass = default("");    //# TODO: Obsolete!!!
        string receiveQueueClass = default(""); //# TODO: Obsolete!!!
        string crcMode @enum("declared", "computed") = default("declared");
        @display("i=block/wheelbarrow");

    gates:
        input appIn @messageKinds(inet::TcpCommandCode);
        input ipIn;
        output appOut @messageKinds(inet::TcpStatusInd);
        output ipOut;
}

