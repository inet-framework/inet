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

//----

Define_Module(TCPTester);

TCPTester::TCPTester(const char *name, cModule *parent) :
  cSimpleModule(name, parent, 0), tcpdump(ev)
{
}

void TCPTester::initialize()
{
    fromASeq = 0;
    fromBSeq = 0;

    const char *script = par("script");
    parseScript(script);
}

void TCPTester::parseScript(const char *script)
{
    const char *s = script;
    while (*s)
    {
        Command cmd;

        // direction
        while (isspace(*s)) s++;
        if (*s=='a' || *s=='A')
            cmd.fromA = true;
        else if (*s=='b' || *s=='B')
            cmd.fromA = false;
        else
            throw new cException("syntax error in script: wrong segment spec");
        s++;

        // seg number
        if (!isdigit(*s))
            throw new cException("syntax error in script: wrong segment spec");
        cmd.segno = atoi(s);
        while (isdigit(*s)) s++;

        // command
        while (isspace(*s)) s++;
        if (!strncmp(s,"delete",6)) {
            cmd.command = CMD_DELETE; s+=6;
        } else if (!strncmp(s,"delay",5)) {
            cmd.command = CMD_COPY; s+=5;
        } else if (!strncmp(s,"copy",4)) {
            cmd.command = CMD_COPY; s+=4;
        } else
            throw new cException("syntax error in script: wrong command");

        // args
        if (cmd.command==CMD_COPY)
        {
            for (;;) {
                while (isspace(*s)) s++;
                if (!*s || *s==';') break;
                double d = strtod(s,&const_cast<char *&>(s));
                cmd.delays.push_back(d);
                while (isspace(*s)) s++;
                if (!*s || *s==';') break;
                if (*s!=',')
                    throw new cException("syntax error in script: error in arg list");
                s++;
            }

        }

        // add command
        commands.push_back(cmd);

        // skip delimiter
        while (isspace(*s)) s++;
        if (!*s) break;
        if (*s!=';')
            throw new cException("syntax error in script: command separator ';' missing");
        s++;
        while (isspace(*s)) s++;
    }
}

void TCPTester::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        TCPSegment *seg = check_and_cast<TCPSegment *>(msg);
        dispatchSegment(seg);
    }
    else
    {
        TCPSegment *seg = check_and_cast<TCPSegment *>(msg);
        bool fromA = msg->arrivedOn("in1");
        processIncomingSegment(seg, fromA);
    }
}

void TCPTester::dump(TCPSegment *seg, bool fromA, const char *comment)
{
    if (ev.disabled()) return;

    char lbl[32];
    sprintf(lbl," %c%03d", fromA ? 'A' : 'B', fromA ? fromASeq : fromBSeq);
    tcpdump.dump(lbl, seg, std::string(fromA?"A":"B"),std::string(fromA?"B":"A"), comment);
}

void TCPTester::dispatchSegment(TCPSegment *seg)
{
    Command *cmd = (Command *)seg->contextPointer();
    bool fromA = cmd->fromA;
    bubble("introducing copy");
    dump(seg, fromA, "introducing copy");
    send(seg, fromA ? "out2" : "out1");
}

void TCPTester::processIncomingSegment(TCPSegment *seg, bool fromA)
{
    int segno = fromA ? ++fromASeq : ++fromBSeq;

    // find entry in script
    Command *cmd = NULL;
    for (CommandVector::iterator i=commands.begin(); i!=commands.end(); ++i)
        if (i->fromA==fromA && i->segno==segno)
            cmd = &(*i);

    if (!cmd)
    {
        // dump & forward
        dump(seg, fromA);
        send(seg, fromA ? "out2" : "out1");
    }
    else if (cmd->command==CMD_DELETE)
    {
        bubble("deleting");
        dump(seg, fromA, "deleting");
        delete seg;
    }
    else if (cmd->command==CMD_COPY)
    {
        bubble("removing original");
        dump(seg, fromA, "removing original");
        for (int i=0; i<cmd->delays.size(); i++)
        {
            double d = cmd->delays[i];
            TCPSegment *segcopy = (TCPSegment *)seg->dup();
            segcopy->setControlInfo(new IPControlInfo(*check_and_cast<IPControlInfo *>(seg->controlInfo())));
            if (d==0)
            {
                bubble("forwarding after 0 delay");
                dump(segcopy, fromA, "introducing copy");
                send(segcopy, fromA ? "out2" : "out1");
            }
            else
            {
                segcopy->setContextPointer(cmd);
                scheduleAt(simTime()+d, segcopy);
            }
        }
        delete seg;
    }
    else
    {
        throw new cException("wrong command code");
    }

}

void TCPTester::finish()
{
    char buf[128];
    sprintf(buf,"tcpdump finished, A:%d B:%d segments",fromASeq,fromBSeq);
    tcpdump.dump("", buf);
}


