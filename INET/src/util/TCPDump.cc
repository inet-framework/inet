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
    cMessage *encapmsg = dgram->getEncapsulatedMsg();
    if (dynamic_cast<TCPSegment *>(encapmsg))
    {
        // if TCP, dump as TCP
        dump(l2r, label, (TCPSegment *)encapmsg, dgram->getSrcAddress().str(), dgram->getDestAddress().str(), comment);
    }
    else
    {
        // some other packet, dump what we can
        std::ostream& out = *outp;

        // seq and time (not part of the tcpdump format)
        char buf[30];
        sprintf(buf,"[%.3f%s] ", SIMTIME_DBL(simTime()), label);
        out << buf;

        // packet class and name
        out << "? " << encapmsg->getClassName() << " \"" << encapmsg->getName() << "\"\n";
    }
}

//FIXME: Temporary hack for Ipv6 support
void TCPDumper::dumpIPv6(bool l2r, const char *label, IPv6Datagram_Base *dgram, const char *comment)
{
    cMessage *encapmsg = dgram->getEncapsulatedMsg();
    if (dynamic_cast<TCPSegment *>(encapmsg))
    {
        // if TCP, dump as TCP
        dump(l2r, label, (TCPSegment *)encapmsg, dgram->getSrcAddress().str(), dgram->getDestAddress().str(), comment);
    }
    else
    {
        // some other packet, dump what we can
        std::ostream& out = *outp;

        // seq and time (not part of the tcpdump format)
        char buf[30];
        sprintf(buf,"[%.3f%s] ", SIMTIME_DBL(simTime()), label);
        out << buf;

        // packet class and name
        out << "? " << encapmsg->getClassName() << " \"" << encapmsg->getName() << "\"\n";
    }
}

void TCPDumper::dump(bool l2r, const char *label, TCPSegment *tcpseg, const std::string& srcAddr, const std::string& destAddr, const char *comment)
{
    std::ostream& out = *outp;

    // seq and time (not part of the tcpdump format)
    char buf[30];
    sprintf(buf,"[%.3f%s] ", SIMTIME_DBL(simTime()), label);
    out << buf;

    // src/dest
    if (l2r)
    {
        out << srcAddr << "." << tcpseg->getSrcPort() << " > ";
        out << destAddr << "." << tcpseg->getDestPort() << ": ";
    }
    else
    {
        out << destAddr << "." << tcpseg->getDestPort() << " < ";
        out << srcAddr << "." << tcpseg->getSrcPort() << ": ";
    }

    // flags
    bool flags = false;
    if (tcpseg->getSynBit()) {flags=true; out << "S";}
    if (tcpseg->getFinBit()) {flags=true; out << "F";}
    if (tcpseg->getPshBit()) {flags=true; out << "P";}
    if (tcpseg->getRstBit()) {flags=true; out << "R";}
    if (!flags) {out << ".";}
    out << " ";

    // data-seqno
    if (tcpseg->getPayloadLength()>0 || tcpseg->getSynBit())
    {
        out << tcpseg->getSequenceNo() << ":" << tcpseg->getSequenceNo()+tcpseg->getPayloadLength();
        out << "(" << tcpseg->getPayloadLength() << ") ";
    }

    // ack
    if (tcpseg->getAckBit())
        out << "ack " << tcpseg->getAckNo() << " ";

    // window
    out << "win " << tcpseg->getWindow() << " ";

    // urgent
    if (tcpseg->getUrgBit())
        out << "urg " << tcpseg->getUrgentPointer() << " ";

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
    sprintf(buf,"[%.3f%s] ", SIMTIME_DBL(simTime()), label);
    out << buf;

    out << msg << "\n";
}

//----

Define_Module(TCPDump);

TCPDump::TCPDump() :
  tcpdump(ev.getOStream())
{
}

void TCPDump::handleMessage(cMessage *msg)
{
    // dump
    if (!ev.isDisabled())
    {
        bool l2r = msg->arrivedOn("a$i");
        if (dynamic_cast<TCPSegment *>(msg))
        {
            tcpdump.dump(l2r, "", (TCPSegment *)msg, std::string(l2r?"A":"B"),std::string(l2r?"B":"A"));
        }
        else
        {
            // search for encapsulated IP[v6]Datagram in it
            cPacket *encapmsg = PK(msg);
            while (encapmsg && dynamic_cast<IPDatagram *>(encapmsg)==NULL && dynamic_cast<IPv6Datagram_Base *>(encapmsg)==NULL)
                encapmsg = encapmsg->getEncapsulatedMsg();
            if (!encapmsg)
            {
                //We do not want this to end in an error if EtherAutoconf messages
                //are passed, so just print a warning. -WEI
                EV << "CANNOT DECODE: packet " << msg->getName() << " doesn't contain either IP or IPv6 Datagram\n";
            }
            else
            {
                if (dynamic_cast<IPDatagram *>(encapmsg))
                    tcpdump.dump(l2r, "", (IPDatagram *)encapmsg);
                else if (dynamic_cast<IPv6Datagram_Base *>(encapmsg))
                    tcpdump.dumpIPv6(l2r, "", (IPv6Datagram_Base *)encapmsg);
                else
                    ASSERT(0); // cannot get here
            }
        }
    }

    // forward
    send(msg, msg->arrivedOn("a$i") ? "b$o" : "a$o");
}

void TCPDump::finish()
{
    tcpdump.dump("", "tcpdump finished");
}

