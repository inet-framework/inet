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

void TCPDumper::dump(bool l2r, const char *label, IPDatagram *dgram, const char *comment)
{
    cMessage *encapmsg = dgram->encapsulatedMsg();
    if (dynamic_cast<TCPSegment *>(encapmsg))
    {
        // if TCP, dump as TCP
        dump(l2r, label, (TCPSegment *)encapmsg, dgram->srcAddress().str(), dgram->destAddress().str(), comment);
    }
    else
    {
        // some other packet, dump what we can
        std::ostream& out = *outp;

        // seq and time (not part of the tcpdump format)
        char buf[30];
        sprintf(buf,"[%.3f%s] ", simulation.simTime(), label);
        out << buf;

        // packet class and name
        out << "? " << encapmsg->className() << " \"" << encapmsg->name() << "\"\n";
    }
}

void TCPDumper::dump(bool l2r, const char *label, TCPSegment *tcpseg, const std::string& srcAddr, const std::string& destAddr, const char *comment)
{
    std::ostream& out = *outp;

    // seq and time (not part of the tcpdump format)
    char buf[30];
    sprintf(buf,"[%.3f%s] ", simulation.simTime(), label);
    out << buf;

    // src/dest
    if (l2r)
    {
        out << srcAddr << "." << tcpseg->srcPort() << " > ";
        out << destAddr << "." << tcpseg->destPort() << ": ";
    }
    else
    {
        out << destAddr << "." << tcpseg->destPort() << " < ";
        out << srcAddr << "." << tcpseg->srcPort() << ": ";
    }

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
        bool l2r = msg->arrivedOn("in1");
        if (dynamic_cast<TCPSegment *>(msg))
        {
            tcpdump.dump(l2r, "", (TCPSegment *)msg, std::string(l2r?"A":"B"),std::string(l2r?"B":"A"));
        }
        else
        {
            // search for encapsulated IPDatagram in it
            cMessage *encapmsg = msg;
            while (encapmsg && dynamic_cast<IPDatagram *>(encapmsg)==NULL)
                encapmsg = encapmsg->encapsulatedMsg();
            if (!encapmsg)
                error("packet %s doesn't contain an IPDatagram", msg->name());
            tcpdump.dump(l2r, "", (IPDatagram *)encapmsg);
        }
    }

    // forward
    send(msg, msg->arrivedOn("in1") ? "out2" : "out1");
}

void TCPDump::finish()
{
    tcpdump.dump("", "tcpdump finished");
}

