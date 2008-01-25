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


//
// An implication of LDP operation model is that LDP host must be able to work
// as both TCP client and server. The functionality of LDPInterface module is
// to implement this capability in a flexible way.
//
// Operations
//
// LDP transport uses TCP as a reliable transport for sessions. When multiple
// LDP sessions are required between two LSRs, there is one TCP session for
// each LDP session. These LDP sessions are established in LDP discovery and
// initialization phases. The establishment must start by Transport Connection
// Establishment.
//
// Transport Connection Establishment between LSR1 and LSR2 through link L
// happens as following: The exchange of discovery message (Hello message)
// results in the creation of a Hello adjacency at LSR1, which binds the link
// (L) and the label spaces LSR1:a and LSR2:b. If the LSR1 does not already
// have an LDP session for the exchange of label spaces LSR1:a and LSR2:b, a
// TCP connection between LSR1 and LSR2 will be likely established. Which LSR
// takes the role of TCP client or TCP server depends on the transport address
// of these LSRs. If the address of LSR1 is greater than that of LSR2, LSR1
// will play the active role (TCP client); otherwise it plays the passive
// role.
//
// To make the Transport Connection Establishment feasible, LDPInterface
// itself emulates the functionalities of a Server Socket when first starting
// up. One exception is that it also sends discovery messages to all
// neighbors. As soon as it receives messages from peers, it will decide the
// role of an active or passive peer to play in that connection. In addition,
// it dynamically creates a new LDPproc module and delegates all communication
// handling functions to this module. After all Connection Establishment have
// been initialized, an LDPInterface has the simple function of message
// dispatching for its plug-in LDPproc modules.
//
// From the above architecture, it is possible for an LSR to be TCP servers in
// some TCP sessions and be TCP clients for the rest of connections with its
// peers.
//
// Collaborations
//
// LDPInterface modules collaborators are LDP module and TCP module. As
// mentioned above, LDPInterface plays the role of a passive server at the
// beginning but then delegates all responsibilities to LDPproc modules. In
// later phase, it dispatches all incoming messages from the lower TCP module
// to appropriate LDPproc modules. In order to differentiate these LDPproc
// modules, the LDPInterface keeps a "hash table" of peer \IP address and the
// id of each LDPproc module. By this method, it knows which LDPproc to
// forward a message to, based on messages sender \IP address.
//
// Usages
//
// LDPInterface requires the input of local address (the loopback address of
// the LSR), timeout value (the maximum of inactive permitted session time),
// and keep-alive time (the refresh time).
//
simple LDPInterface

{
    parameters:
        string local_addr;
        volatile double timeout;
        volatile double keepAliveTime;

    gates:
        input from_tcp;
        input from_ldp;
        output to_tcp;
        output to_ldp;

}
