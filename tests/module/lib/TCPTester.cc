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


#include "TCPTester.h"
#include "IPv4ControlInfo.h"

TCPTesterBase::TCPTesterBase() : cSimpleModule()
{
}

void TCPTesterBase::initialize()
{
    fromASeq = 0;
    fromBSeq = 0;
    tcpdump.setOutStream(ev.getOStream());
}

void TCPTesterBase::dump(TCPSegment *seg, bool fromA, const char *comment)
{
    if (ev.isDisabled()) return;

    char lbl[32];
    sprintf(lbl," %c%03d", fromA ? 'A' : 'B', fromA ? fromASeq : fromBSeq);
    tcpdump.tcpDump(fromA, lbl, seg, std::string(fromA?"A":"B"),std::string(fromA?"B":"A"), comment);
}

void TCPTesterBase::finish()
{
    char buf[128];
    sprintf(buf,"tcpdump finished, A:%d B:%d segments",fromASeq,fromBSeq);
    tcpdump.dump("", buf);
}



//---

Define_Module(TCPScriptableTester);

void TCPScriptableTester::initialize()
{
    TCPTesterBase::initialize();

    const char *script = par("script");
    parseScript(script);
}

void TCPScriptableTester::parseScript(const char *script)
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
            throw cRuntimeError("syntax error in script: wrong segment spec");
        s++;

        // seg number
        if (!isdigit(*s))
            throw cRuntimeError("syntax error in script: wrong segment spec");
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
            throw cRuntimeError("syntax error in script: wrong command");

        // args
        if (cmd.command==CMD_COPY)
        {
            for (;;)
            {
                while (isspace(*s)) s++;
                if (!*s || *s==';') break;
                double d = strtod(s,&const_cast<char *&>(s));
                cmd.delays.push_back(d);
                while (isspace(*s)) s++;
                if (!*s || *s==';') break;
                if (*s!=',')
                    throw cRuntimeError("syntax error in script: error in arg list");
                s++;
            }
        }

        // add command
        commands.push_back(cmd);

        // skip delimiter
        while (isspace(*s)) s++;
        if (!*s) break;
        if (*s!=';')
            throw cRuntimeError("syntax error in script: command separator ';' missing");
        s++;
        while (isspace(*s)) s++;
    }
}

void TCPScriptableTester::handleMessage(cMessage *msg)
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

void TCPScriptableTester::dispatchSegment(TCPSegment *seg)
{
    Command *cmd = (Command *)seg->getContextPointer();
    bool fromA = cmd->fromA;
    bubble("introducing copy");
    dump(seg, fromA, "introducing copy");
    send(seg, fromA ? "out2" : "out1");
}

void TCPScriptableTester::processIncomingSegment(TCPSegment *seg, bool fromA)
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
        for (unsigned int i=0; i<cmd->delays.size(); i++)
        {
            simtime_t d = cmd->delays[i];
            TCPSegment *segcopy = (TCPSegment *)seg->dup();
            segcopy->setControlInfo(new IPv4ControlInfo(*check_and_cast<IPv4ControlInfo *>(seg->getControlInfo())));
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
        throw cRuntimeError("wrong command code");
    }
}

//------

Define_Module(TCPRandomTester);

void TCPRandomTester::initialize()
{
    TCPTesterBase::initialize();

    pdelete = par("pdelete");
    pdelay = par("pdelay");
    pcopy = par("pcopy");
    numCopies = &par("numCopies");
    delay = &par("delayValue");
}

void TCPRandomTester::handleMessage(cMessage *msg)
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

void TCPRandomTester::dispatchSegment(TCPSegment *seg)
{
    bool fromA = (bool)seg->getContextPointer();
    bubble("introducing copy");
    dump(seg, fromA, "introducing copy");
    send(seg, fromA ? "out2" : "out1");
}

void TCPRandomTester::processIncomingSegment(TCPSegment *seg, bool fromA)
{
    if (fromA) ++fromASeq; else ++fromBSeq;

    // decide what to do
    double x = dblrand();
    if (x<=pdelete)
    {
        bubble("deleting");
        dump(seg, fromA, "deleting");
        delete seg;
    }
    else if (x-=pdelete, x<=pdelay)
    {
        bubble("delay: removing original");
        dump(seg, fromA, "delay: removing original");
        double d = delay->doubleValue();
        seg->setContextPointer((void*)fromA);
        scheduleAt(simTime()+d, seg);
    }
    else if (x-=pdelay, x<=pcopy)
    {
        bubble("copy: removing original");
        dump(seg, fromA, "copy: removing original");
        int n = numCopies->longValue();
        for (int i=0; i<n; i++)
        {
            double d = delay->doubleValue();
            TCPSegment *segcopy = (TCPSegment *)seg->dup();
            segcopy->setControlInfo(new IPv4ControlInfo(*check_and_cast<IPv4ControlInfo *>(seg->getControlInfo())));
            segcopy->setContextPointer((void *)fromA);
            scheduleAt(simTime()+d, segcopy);
        }
        delete seg;
    }
    else
    {
        // dump & forward
        dump(seg, fromA);
        send(seg, fromA ? "out2" : "out1");
    }
}

