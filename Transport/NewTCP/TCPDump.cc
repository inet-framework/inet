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

TCPDumper::TCPDumper(std::ostream& out)
{
    outp = &out;
}

void TCPDumper::dump(IPDatagram *dgram)
{
    TCPSegment *tcpseg = check_and_cast<TCPSegment *>(dgram->encapsulatedMsg());
    dump(tcpseg, dgram->srcAddress().str(), dgram->destAddress().str());
}

void TCPDumper::dump(TCPSegment *tcpseg, const std::string& srcAddr, const std::string& destAddr)
{
    std::ostream& out = *outp;

    // src/dest
    out << srcAddr << "." << tcpseg->srcPort() << " > ";
    out << destAddr << "." << tcpseg->destPort() << ": ";

    // flags
    bool flags = false;
    if (tcpseg->synBit()) {flags=true; out << "S";}
    if (tcpseg->finBit()) {flags=true; out << "F";}
    if (tcpseg->pshBit()) {flags=true; out << "P";}
    if (tcpseg->rstBit()) {flags=true; out << "R";}
    if (!flags) {out << ".";}
    out << " ";

    // data-seqno
    if (tcpseg->payloadLength()>0 || tcpseg->synBit())
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

Define_Module(TCPDump);

TCPDump::TCPDump(const char *name, cModule *parent) :
  cSimpleModule(name, parent, 0), tcpdump(ev)
{
}

void TCPDump::handleMessage(cMessage *msg)
{
    // dump
    if (dynamic_cast<IPDatagram *>(msg))
    {
        tcpdump.dump((IPDatagram *)msg);
    }
    else if (dynamic_cast<TCPSegment *>(msg))
    {
        bool dir = msg->arrivedOn("in1");
        tcpdump.dump((TCPSegment *)msg, std::string(dir ? "first" : "second"),
                                        std::string(dir ? "second" : "first")
                    );
    }

    // forward
    send(msg, msg->arrivedOn("in1") ? "out2" : "out1");
}

