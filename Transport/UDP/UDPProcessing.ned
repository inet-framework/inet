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
// UDP protocol implementation.
//
// The UDP protocol header is represented by the class UDPPacket.
//
// <b>Communication with clients (applications)</b>
//
// The module can (should) be connected to several applications.
// For sending an UDP packet, the application should attach an UDPControlInfo
// object to the payload, and send it to UDPProcessing.
// UDPProcessing will also attach an UDPControlInfo object to any payload
// message in sends up to the application.
//
// For receiving UDP packets, the connected applications should have
// "local_port" parameters which specify the port number they're interested
// in. One port can only belong to one app. Packets that don't match
// any app will be dropped.
//
// If there's only one app and without "local_port" parameter, that app
// will receive all incoming packets regardless of destination port number.
//
// <b>Communication with the \IP layer</b>
//
// The UDP model relies on sending and receiving IPControlInfo objects
// attached to UDPPacket objects as control info
// (see cMessage::setControlInfo()).
//
simple UDPProcessing
    gates:
        in: from_application[];
        in: from_ip;
        out: to_application[];
        out: to_ip;
endsimple


