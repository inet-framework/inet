//
// Copyright (C) 2006 Sam Jansen, Andras Varga
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
//

package inet.transportlayer.tcp_nsc;

import inet.transportlayer.contract.ITcp;

//
// TCP model based on the Network Simulation Cradle by Sam Jansen.
// The NSC is available on the http://research.wand.net.nz/software/nsc.php page.
// You must read the inet/3dparty/README before use this TCP implementation.
// This model is compatible with both IPv4 (~Ipv4) and IPv6 (~Ipv6).
// The TCP_TRANSFER_OBJECT data transfer mode isn't implemented yet.
// See the ~ITcp for the TCP layer general informations.
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
simple TcpNsc like ITcp
{
    parameters:
        // full library name of the tcp stack implementation
        string stackName @choice("liblinux2.6.10.so","liblinux2.6.18.so","liblinux2.6.26.so","liblwip.so","libopenbsd3.5.so","libfreebsd5.3.so") = default("liblinux2.6.26.so");
        int stackBufferSize = default(80000); // the size of buffer used when communicating with the specified NSC stack
        string sendQueueClass = default("");    //# TODO Obsolete!!!
        string receiveQueueClass = default(""); //# TODO Obsolete!!!
        string crcMode @enum("declared", "computed") = default("declared");
        @display("i=block/wheelbarrow");

    gates:
        input appIn @messageKinds(inet::TcpCommandCode);
        input ipIn;
        output appOut @messageKinds(inet::TcpStatusInd);
        output ipOut;
}

