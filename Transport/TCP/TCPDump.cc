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
#include "IPControlInfo_m.h"


TCPDumper::TCPDumper(std::ostream& out)
{
    outp = &out;
}

void TCPDumper::dump(const char *label, IPDatagram *dgram, const char *comment)
{
    TCPSegment *tcpseg = check_and_cast<TCPSegment *>(dgram->encapsulatedMsg());
    dump(label, tcpseg, dgram->srcAddress().str(), dgram->destAddress().str(), comment);
}

void TCPDumper::dump(const char *label, TCPSegment *tcpseg, const std::string& srcAddr, const std::string& destAddr, const char *comment)
{
    std::ostream& out = *outp;

    // seq and time (not part of the tcpdump format)
    char buf[30];
    sprintf(buf,"[%.3f%s] ", simulation.simTime(), label);
    out << buf;

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

    // options (not supported by TCPSegment yet)

    // comment
    if (comment)
        out << "# " << comment;

    out << endl;
}

void TCPDumper::dump(const char *label, const char *msg)
{
    std::ostream& out = *outp;

    // seq and time (not part of the tcpdump format)
    char buf[30];
    sprintf(buf,"[%.3f%s] ", simulation.simTime(), label);
    out << buf;

    out << msg << "\n";
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
    if (!ev.disabled())
    {
        if (dynamic_cast<IPDatagram *>(msg))
        {
            tcpdump.dump("", (IPDatagram *)msg);
        }
        else if (dynamic_cast<TCPSegment *>(msg))
        {
            bool dir = msg->arrivedOn("in1");
            tcpdump.dump("", (TCPSegment *)msg, std::string(dir?"A":"B"),std::string(dir?"B":"A"));
        }
    }

    // forward
    send(msg, msg->arrivedOn("in1") ? "out2" : "out1");
}

void TCPDump::finish()
{
    tcpdump.dump("", "tcpdump finished");
}

