//
// Copyright (C) 2004 Andras Varga
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


#include "TCPDump.h"

TCPDump::TCPDump(std::ostream& out)
{
    outp = &out;
}

void TCPDump::dump(IPDatagram *dgram)
{
    TCPSegment *tcpseg = check_and_cast<TCPSegment *>(dgram->encapsulatedMsg());
    dump(tcpseg, dgram->srcAddress(), dgram->destAddress());
}

void TCPDump::dump(TCPSegment *tcpseg, IPAddress srcAddr, IPAddress destAddr)
{
    std::ostream& out = *outp;

    // src/dest
    out << tcpseg->srcAddress().c_str() << "." << tcpseg->srcPort() << " > ";
    out << tcpseg->destAddress().c_str() << "." << tcpseg->destPort() << ": ";

    // flags
    bool flags = false;
    if (tcpseg->synBit()) {flags=true; out << "S";}
    if (tcpseg->finBit()) {flags=true; out << "F";}
    if (tcpseg->pshBit()) {flags=true; out << "P";}
    if (tcpseg->rstBit()) {flags=true; out << "R";}
    if (!flags) {out << ".";}
    out << " ";

    // data-seqno
    out << tcpseg->sequenceNo() << ":" << tcpseg->sequenceNo()+tcpseg->payloadLength()-1;
    out << "(" << tcpseg->payloadLength() << ") ";

    // ack
    out << tcpseg->ackNo() << " ";

    // window
    // urgent
    // options
}
