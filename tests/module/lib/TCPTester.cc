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

#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"

namespace inet {

namespace tcp {

TCPTesterBase::TCPTesterBase() : cSimpleModule()
{
}

void TCPTesterBase::initialize()
{
    fromASeq = 0;
    fromBSeq = 0;
    tcpdump.setOutStream(EVSTREAM);
    tcpdump.setVerbose(true);
}

void TCPTesterBase::dump(const Ptr<const inet::tcp::TcpHeader>& seg, int tcpLength, bool fromA, const char *comment)
{
    if (getEnvir()->isExpressMode())
        return;

    char lbl[32];
    sprintf(lbl," %c%03d", fromA ? 'A' : 'B', fromA ? fromASeq : fromBSeq);
    std::ostringstream out;
    tcpdump.setOutStream(out);
    tcpdump.tcpDump(fromA, lbl, seg, tcpLength, std::string(fromA ? "A":"B"),std::string(fromA ? "B":"A"), comment);
    EV_DEBUG_C("testing") << out.str();
    tcpdump.setOutStream(EVSTREAM);
}

void TCPTesterBase::finish()
{
    char buf[128];
    sprintf(buf,"tcpdump finished, A:%d B:%d segments",fromASeq,fromBSeq);
    std::ostringstream out;
    tcpdump.setOutStream(out);
    tcpdump.dump("", buf);
    EV_DEBUG_C("testing") << out.str();
    tcpdump.setOutStream(EVSTREAM);
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
    Packet *seg = check_and_cast<Packet *>(msg);
    if (msg->isSelfMessage())
    {
        dispatchSegment(seg);
    }
    else if (msg->isPacket())
    {
        bool fromA = msg->arrivedOn("in1");
        processIncomingSegment(seg, fromA);
    }
    else
    {
        throw cRuntimeError("Unknown message");
    }
}

void TCPScriptableTester::dispatchSegment(Packet *pk)
{
    const auto& seg = pk->peekAtFront<TcpHeader>();
    if (seg == nullptr)
        throw cRuntimeError("Unknown message");
    Command *cmd = (Command *)pk->getContextPointer();
    bool fromA = cmd->fromA;
    bubble("introducing copy");
    dump(seg, pk->getByteLength(), fromA, "introducing copy");
    send(pk, fromA ? "out2" : "out1");
}

void TCPScriptableTester::processIncomingSegment(Packet *pk, bool fromA)
{
    const auto& seg = pk->peekAtFront<TcpHeader>();
    if (seg == nullptr)
        throw cRuntimeError("Unknown message");
    int segno = fromA ? ++fromASeq : ++fromBSeq;
//    const Protocol *protInd = pk->getTag<ProtocolInd>()->getProtocol();
//    const Protocol *protReq = pk->getTag<ProtocolReq>()->getProtocol();
//    pk->addTagIfAbsent<ProtocolReq>()->setProtocol(protInd);
//    pk->addTagIfAbsent<ProtocolInd>()->setProtocol(protReq);
    pk->addTagIfAbsent<L3AddressInd>()->setSrcAddress(pk->getTag<L3AddressReq>()->getSrcAddress());
    pk->addTagIfAbsent<L3AddressInd>()->setDestAddress(pk->getTag<L3AddressReq>()->getDestAddress());
    pk->removeTag<L3AddressReq>();

    // find entry in script
    Command *cmd = NULL;
    for (CommandVector::iterator i=commands.begin(); i!=commands.end(); ++i)
        if (i->fromA==fromA && i->segno==segno)
            cmd = &(*i);

    if (!cmd)
    {
        // dump & forward
        dump(seg, pk->getByteLength(), fromA);
        send(pk, fromA ? "out2" : "out1");
    }
    else if (cmd->command==CMD_DELETE)
    {
        bubble("deleting");
        dump(seg, pk->getByteLength(), fromA, "deleting");
        delete pk;
    }
    else if (cmd->command==CMD_COPY)
    {
        bubble("removing original");
        dump(seg, pk->getByteLength(), fromA, "removing original");
        for (unsigned int i=0; i<cmd->delays.size(); i++)
        {
            simtime_t d = cmd->delays[i];
            Packet *segcopy = pk->dup();

            if (d==0)
            {
                bubble("forwarding after 0 delay");
                dump(seg, pk->getByteLength(), fromA, "introducing copy");
                send(segcopy, fromA ? "out2" : "out1");
            }
            else
            {
                segcopy->setContextPointer(cmd);
                scheduleAt(simTime()+d, segcopy);
            }
        }
        delete pk;
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
    Packet *seg = check_and_cast<Packet *>(msg);
    if (msg->isSelfMessage())
    {
        dispatchSegment(seg);
    }
    else if (msg->isPacket())
    {
        bool fromA = msg->arrivedOn("in1");
        processIncomingSegment(seg, fromA);
    }
    else
    {
        throw cRuntimeError("Unknown message");
    }
}

void TCPRandomTester::dispatchSegment(Packet *pk)
{
    const auto& seg = pk->peekAtFront<TcpHeader>();
    if (seg == nullptr)
        throw cRuntimeError("Unknown message");
    bool fromA = (bool)pk->getContextPointer();
    bubble("introducing copy");
    dump(seg, pk->getByteLength(), fromA, "introducing copy");
    send(pk, fromA ? "out2" : "out1");
}

void TCPRandomTester::processIncomingSegment(Packet *pk, bool fromA)
{
    const auto& seg = pk->peekAtFront<TcpHeader>();
    if (seg == nullptr)
        throw cRuntimeError("Unknown message");
    if (fromA) ++fromASeq; else ++fromBSeq;

//    const Protocol *protInd = pk->getTag<ProtocolInd>()->getProtocol();
//    const Protocol *protReq = pk->getTag<ProtocolReq>()->getProtocol();
//    pk->addTagIfAbsent<ProtocolReq>()->setProtocol(protInd);
//    pk->addTagIfAbsent<ProtocolInd>()->setProtocol(protReq);
    pk->addTagIfAbsent<L3AddressInd>()->setSrcAddress(pk->getTag<L3AddressReq>()->getSrcAddress());
    pk->addTagIfAbsent<L3AddressInd>()->setDestAddress(pk->getTag<L3AddressReq>()->getDestAddress());
    pk->removeTag<L3AddressReq>();

    // decide what to do
    double x = dblrand();
    if (x<=pdelete)
    {
        bubble("deleting");
        dump(seg, pk->getByteLength(), fromA, "deleting");
        delete pk;
    }
    else if (x-=pdelete, x<=pdelay)
    {
        bubble("delay: removing original");
        dump(seg, pk->getByteLength(), fromA, "delay: removing original");
        double d = *delay;
        pk->setContextPointer((void*)fromA);
        scheduleAt(simTime()+d, pk);
    }
    else if (x-=pdelay, x<=pcopy)
    {
        bubble("copy: removing original");
        dump(seg, pk->getByteLength(), fromA, "copy: removing original");
        int n = *numCopies;
        for (int i=0; i<n; i++)
        {
            double d = *delay;
            Packet *segcopy = pk->dup();
            segcopy->setContextPointer((void *)fromA);
            scheduleAt(simTime()+d, segcopy);
        }
        delete pk;
    }
    else
    {
        // dump & forward
        dump(seg, pk->getByteLength(), fromA);
        send(pk, fromA ? "out2" : "out1");
    }
}

} // namespace tcp

} // namespace inet

