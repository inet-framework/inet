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
//# FIXME edit comment!
//
// Initialisation:
//   error check validity initialized as parameter
//
// Responsibilities:
//   initialize with port-to-application-table
//
//   receive message with encoded ports and IP addresses as parameters from application:
//   create UDPPacket from it.
//   encapsulate it into IPInterfacePacket
//   pass it on to the IP layer
//
//   receive IPInterfacePacket from IP Layer:
//   decapsulate UDPPacket
//   check biterror if checksum enabled
//       -> throw away if biterror found
//   map port to correct application-gate
//   create UDPInterfacePacket containing address/port info
//   pass UDPInterfacePacket to the selected gate
//
simple UDPProcessing
    gates:
        in: from_application[];
        in: from_ip;
        out: to_application[];
        out: to_ip;
endsimple


