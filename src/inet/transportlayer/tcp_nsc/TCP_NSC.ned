//
// Copyright (C) 2006 Sam Jansen, Andras Varga
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

package inet.transportlayer.tcp_nsc;

import inet.transportlayer.contract.ITCP;

//
// TCP model based on the Network Simulation Cradle by Sam Jansen.
// The NSC is available on the http://research.wand.net.nz/software/nsc.php page.
// You must read the inet/3dparty/README before use this TCP implementation.
// This model is compatible with both IPv4 (~IPv4) and ~IPv6.
// The TCP_TRANSFER_OBJECT data transfer mode isn't implemented yet.
// See the ~ITCP for the TCP layer general informations.
//
// <b>Settings</b>
//
// stackName: You can select a TCP implementation with the stackName parameter
// (On the 64 bit systems, the liblinux2.6.26.so and liblinux2.6.16.so are available only).
//
// stackBufferSize: The buffer size value for selected TCP implementation.
// The NSC sets the wmem_max, rmem_max, tcp_rmem, tcp_wmem parameters to this value
// on linux TCP implementations. For details, you can see the NSC documentation.
//
simple TCP_NSC like ITCP
{
    parameters:
        // full library name of the tcp stack implementation
        string stackName @choice("liblinux2.6.10.so","liblinux2.6.18.so","liblinux2.6.26.so","liblwip.so","libopenbsd3.5.so","libfreebsd5.3.so") = default("liblinux2.6.26.so");
        int stackBufferSize = default(80000); // the size of buffer used when communicating with the specified NSC stack
        string sendQueueClass = default("");    // Obsolete!!!
        string receiveQueueClass = default(""); // Obsolete!!!
        @display("i=block/wheelbarrow");
    gates:
        input appIn[];
        input ipIn;
        output appOut[];
        output ipOut;
}

