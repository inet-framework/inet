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
    out << srcAddr.c_str() << "." << tcpseg->srcPort() << " > ";
    out << destAddr.c_str() << "." << tcpseg->destPort() << ": ";

    // flags
    bool flags = false;
    if (tcpseg->synBit()) {flags=true; out << "S";}
    if (tcpseg->finBit()) {flags=true; out << "F";}
    if (tcpseg->pshBit()) {flags=true; out << "P";}
    if (tcpseg->rstBit()) {flags=true; out << "R";}
    if (!flags) {out << ".";}
    out << " ";

    // data-seqno
    if (tcpseg->payloadLength()>0)
    {
        out << tcpseg->sequenceNo() << ":" << tcpseg->sequenceNo()+tcpseg->payloadLength();
        out << "(" << tcpseg->payloadLength() << ") ";
    }

    // ack
    if (tcpseg->ackBit())
        out << "ack " << tcpseg->ackNo() << " ";

    // window
    out << "win " << tcpseg->window() << " ";

    // urgent
    if (tcpseg->urgBit())
        out << "urg " << tcpseg->urgentPointer() << " ";

    // options
    //...
    out << endl;
}

//----

TCPDumpModule::TCPDumpModule(const char *name, cModule *parent) :
  cSimpleModule(name, parent), tcpdump(ev)
{
}

void TCPDumpModule::handleMessage(cMessage *msg)
{
    // dump
    IPDatagram *dgram = check_and_cast<IPDatagram *>(msg);
    tcpdump.dump(dgram);

    // forward
    int ingateindex = msg->arrivalGate()->index();
    send(msg, "out", 1-ingateindex);
}

