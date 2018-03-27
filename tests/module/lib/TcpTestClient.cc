//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// Copyright 2004 Andras Varga
//

#include <vector>
#include <string>

#include "inet/common/INETDefs.h"

#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

/**
 * TCP client application for testing the TCP model.
 */
class INET_API TcpTestClient : public cSimpleModule
{
  protected:
    struct Command
    {
        simtime_t tSend;
        int numBytes;
    };
    typedef std::list<Command> Commands;
    Commands commands;

    enum { TEST_OPEN, TEST_SEND, TEST_CLOSE };

    int ctr;

    TcpSocket socket;

    // statistics
    int64_t rcvdBytes;
    int rcvdPackets;

  protected:
    void parseScript(const char *script);
    std::string makeMsgName();
    void handleSelfMessage(cMessage *msg);
    void scheduleNextSend();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

Define_Module(TcpTestClient);


void TcpTestClient::parseScript(const char *script)
{
    const char *s = script;
    while (*s)
    {
        Command cmd;

        // parse time
        while (isspace(*s)) s++;
        if (!*s || *s==';') break;
        const char *s0 = s;
        cmd.tSend = strtod(s,&const_cast<char *&>(s));
        if (s==s0)
            throw cRuntimeError("syntax error in script: simulation time expected");

        // parse number of bytes
        while (isspace(*s)) s++;
        if (!isdigit(*s))
            throw cRuntimeError("syntax error in script: number of bytes expected");
        cmd.numBytes = atoi(s);
        while (isdigit(*s)) s++;

        // add command
        commands.push_back(cmd);

        // skip delimiter
        while (isspace(*s)) s++;
        if (!*s) break;
        if (*s!=';')
            throw cRuntimeError("syntax error in script: separator ';' missing");
        s++;
        while (isspace(*s)) s++;
    }
}

std::string TcpTestClient::makeMsgName()
{
    char buf[40];
    sprintf(buf,"data-%d", ++ctr);
    return std::string(buf);
}

void TcpTestClient::initialize()
{
    rcvdBytes = 0;
    rcvdPackets = 0;

    // parameters
    simtime_t tOpen = par("tOpen");
    Command cmd;
    cmd.tSend = par("tSend");
    cmd.numBytes = par("sendBytes");
    simtime_t tClose = par("tClose");
    const char *script = par("sendScript");

    if (cmd.numBytes > 0)
        commands.push_back(cmd);

    parseScript(script);
    if (cmd.numBytes > 0 && commands.size() > 1)
        throw cRuntimeError("cannot use both sendScript and tSend+sendBytes");

    socket.setOutputGate(gate("socketOut"));

    ctr = 0;

    scheduleAt(tOpen, new cMessage("Open", TEST_OPEN));
    if (tClose > 0)
        scheduleAt(tClose, new cMessage("Close", TEST_CLOSE));
}

void TcpTestClient::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        handleSelfMessage(msg);
        return;
    }

    //EV << fullPath() << ": received " << msg->name() << ", " << msg->byteLength() << " bytes\n";
    if (msg->getKind()==TCP_I_DATA || msg->getKind()==TCP_I_URGENT_DATA)
    {
        rcvdPackets++;
        rcvdBytes += PK(msg)->getByteLength();
    }
    socket.processMessage(msg);
}

void TcpTestClient::handleSelfMessage(cMessage *msg)
{
    switch (msg->getKind())
    {
        case TEST_OPEN:
        {
            const char *localAddress = par("localAddress");
            int localPort = par("localPort");
            const char *connectAddress = par("connectAddress");
            int connectPort = par("connectPort");

            socket.bind(*localAddress ? L3Address(localAddress) : L3Address(), localPort);

            if (par("active"))
                socket.connect(L3Address(connectAddress), connectPort);
            else
                socket.listenOnce();
            scheduleNextSend();
            delete msg;
            break;
        }
        case TEST_SEND:
            socket.send(check_and_cast<Packet *>(msg));
            scheduleNextSend();
            break;
        case TEST_CLOSE:
            socket.close();
            delete msg;
            break;
        default:
            throw cRuntimeError("Unknown self message!");
            break;
    }
}

void TcpTestClient::scheduleNextSend()
{
    if (commands.empty())
        return;
    Command cmd = commands.front();
    commands.pop_front();
    Packet *msg = new Packet(makeMsgName().c_str(), TEST_SEND);
    const auto& bytes = makeShared<ByteCountChunk>(B(cmd.numBytes));
    msg->insertAtBack(bytes);
    scheduleAt(cmd.tSend, msg);
}

void TcpTestClient::finish()
{
    EV << getFullPath() << ": received " << rcvdBytes << " bytes in " << rcvdPackets << " packets\n";
}

} // namespace inet

