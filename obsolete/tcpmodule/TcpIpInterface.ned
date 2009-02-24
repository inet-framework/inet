//
//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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
//
//
simple Tcp2Ip
{
    //parameters:
    gates:
        input in;
        output out;
}


simple Ip2Tcp
{
    //parameters:
    gates:
        input in;
        output out;
}


module TcpIpInterface
{
    parameters:
        @display("i=prot3");
    gates:
        input from_tcp;
        input from_ip;
        output to_tcp;
        output to_ip;
    submodules:
        tcp2Ip: Tcp2Ip {};
        ip2Tcp: Ip2Tcp {};
    connections:
        from_tcp --> tcp2Ip.in;
        tcp2Ip.out --> to_ip;
        from_ip --> ip2Tcp.in;
        ip2Tcp.out --> to_tcp;
}
