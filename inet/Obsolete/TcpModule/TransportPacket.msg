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
// Base class for UDPPacket and TCPPacket
//# FIXME must die!!! we don't need common base class for these two!
//
message TransportPacket extends cPacket
{
    properties:
        omitGetVerb = true;
    fields:
        int sourcePort = -1;
        int destinationPort = -1;
        int msgKind;  // FIXME what the hell is this??
};


