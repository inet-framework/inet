//
//
// Copyright (C) 2000 Institut fuer Nachrichtentechnik, Universitaet Karlsruhe
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
// TCP protocol implementation.
//
// The application can send the following commands to TcpModule:
//  - <tt>TCP_C_OPEN_PASSIVE</tt>: server app ready to accept incoming connection
//    (like LISTEN)
//  - <tt>TCP_C_OPEN_ACTIVE</tt>: open a connection (server must have already
//    issued passive open)
//  - <tt>TCP_C_SEND</tt>
//  - <tt>TCP_C_RECEIVE</tt>
//  - <tt>TCP_C_CLOSE</tt>: close connection
//  - <tt>TCP_C_ABORT</tt>
//  - <tt>TCP_C_STATUS</tt>
//
// The following indications are sent from TcpModule to the application:
//  - <tt>TCP_I_NONE</tt>
//  - <tt>TCP_I_SEG_FWD</tt>
//  - <tt>TCP_I_RCVD_SYN</tt>
//  - <tt>TCP_I_ESTAB</tt>
//  - <tt>TCP_I_CLOSE_WAIT</tt>
//  - <tt>TCP_I_CLOSED</tt>
//  - <tt>TCP_I_ABORTED</tt>
//
// The commands and indications are represented by messages, with the
// message kind set to one of the above enum values. Additional parameters
// (added via <tt>cPar</tt>s):
//  - <tt>src_port</tt>, <tt>src_addr</tt>, <tt>dest_port</tt>,
//    <tt>dest_addr</tt> (all as <tt>int</tt>s)
//  - <tt>rec_pks</tt>: number of packets the application is ready to receive
//  - <tt>num_bit_req</tt>: number of bits the application is ready to receive (?)
//  - <tt>tcp_conn_id</tt>: see below
//
// <tt>tcp_conn_id</tt> is an identifier which has the same value on the
// client and server side. In fact, it does <b>not</b> identify the connection
// for Tcpmodule (it uses the src/dest addresses and ports for this purpose,
// in every case), so perhaps it was meant for use by the applications.
//
// <tt>tcp_conn_id</tt> is assigned by the server application, when issuing
// the passive open call. The client issues the active open without
// a <tt>tcp_conn_id</tt> (i.e. with a dummy one), and will learn the
// connection id from the <tt>TCP_I_ESTAB</tt> indication.
// The server is free to use any identifier; it might be handy to
// use e.g. the module id of the process (dynamically created module)
// that handles the connection.
//
simple TcpModule
{
    parameters:
        //# the following parameters are not used so far.
        //# TCPSlowStart   : bool,
        bool TCPDelayedAck;
        bool TCPFastRetrans;
        bool TCPFastRecovery;
        bool TCPNewReno;
        volatile double mss; // maximum segment size
      //#debug: bool;
        @display("i=block/wheelbarrow");
    gates:
        input from_appl;
        input from_ip;
        output to_appl;
        output to_ip;
}





