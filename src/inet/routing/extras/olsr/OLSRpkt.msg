// ***************************************************************************
// *   Copyright (C) 2008 Alfonso Ariza Quintana aarizaq@uma.es              *
// *                                                                         *
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// *   This program is distributed in the hope that it will be useful,       *
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
// *   GNU General Public License for more details.                          *
// *                                                                         *
// *   You should have received a copy of the GNU General Public License     *
// *   along with this program; if not, write to the                         *
// *   Free Software Foundation, Inc.,                                       *
// *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
// ***************************************************************************

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.common.L3Address;
import inet.routing.extras.olsr.OlsrMsg;

//
// Represents an OLSR packet
//
// Uses the following cMessage fields:
//    - length () / setLength(err)  in bits
//    - hasBitError() / setBitError()
//    - timestamp() / setTimestamp (simtime) used in timestamp option
//
// Additional length fields defined in this class are in bytes
// (totalLength()=length()/8 and header_length), or 8 bytes (fragment_offset).
//
// Only one of the option fields can exist at a time.
//
//class noncobject OLSR_msg;

namespace inet::inetmanet;

class OLSR_pkt extends FieldsChunk
{
    @omitGetVerb(true);
    bool reduceFuncionality=false;
    short pkt_seq_num;          ///< Packet sequence number.
    long sn;                    /// CapProbe packet sequence number
    double send_time;           /// Packet sending timestamp
    OLSR_msg msg[];             ///< Packet body.
}
