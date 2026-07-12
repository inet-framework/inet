//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <cstring>
#include <vector>
#include <string>

#include "inet/common/INETDefs.h"

#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/transportlayer/contract/tcp/TcpSendEorTag_m.h"
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
        bool eor = false; // Workstream H1 (MSG_EOR): optional "eor" keyword after numBytes in sendScript
    };
    typedef std::list<Command> Commands;
    Commands commands;
    Commands commands2; // for the optional second connection (socket2)
    Commands commands3; // for the optional third connection (socket3)

    enum { TEST_OPEN, TEST_SEND, TEST_CLOSE, TEST_OPEN2, TEST_SEND2, TEST_OPEN3, TEST_SEND3 };

    int ctr;

    TcpSocket socket;
    TcpSocket socket2; // optional second, independent connection from the same app/Tcp module -- see active2/tOpen2
    TcpSocket socket3; // optional third, independent connection from the same app/Tcp module -- see active3/tOpen3

    // statistics
    int64_t rcvdBytes;
    int rcvdPackets;

  protected:
    void parseScript(const char *script);
    std::string makeMsgName();
    void handleSelfMessage(cMessage *msg);
    void scheduleNextSend();
    void scheduleNextSend2();
    void scheduleNextSend3();

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

        // Workstream H1 (MSG_EOR): optional "eor" keyword right after the byte
        // count marks this command's SEND as a record boundary.
        while (isspace(*s)) s++;
        if (strncmp(s, "eor", 3) == 0 && !isalnum(s[3])) {
            cmd.eor = true;
            s += 3;
        }

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
    socket2.setOutputGate(gate("socketOut"));
    socket3.setOutputGate(gate("socketOut"));

    ctr = 0;

    scheduleAt(tOpen, new cMessage("Open", TEST_OPEN));
    if (tClose > 0)
        scheduleAt(tClose, new cMessage("Close", TEST_CLOSE));

    simtime_t tOpen2 = par("tOpen2");
    if (tOpen2 >= SIMTIME_ZERO) {
        Command cmd2;
        cmd2.tSend = par("tSend2");
        cmd2.numBytes = par("sendBytes2");
        if (cmd2.numBytes > 0)
            commands2.push_back(cmd2);
        scheduleAt(tOpen2, new cMessage("Open2", TEST_OPEN2));
    }

    simtime_t tOpen3 = par("tOpen3");
    if (tOpen3 >= SIMTIME_ZERO) {
        Command cmd3;
        cmd3.tSend = par("tSend3");
        cmd3.numBytes = par("sendBytes3");
        if (cmd3.numBytes > 0)
            commands3.push_back(cmd3);
        scheduleAt(tOpen3, new cMessage("Open3", TEST_OPEN3));
    }
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
    if (socket2.belongsToSocket(msg))
        socket2.processMessage(msg);
    else if (socket3.belongsToSocket(msg))
        socket3.processMessage(msg);
    else
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

            socket.setAutoRead(par("autoRead"));

            if (par("active"))
                socket.connect(L3Address(connectAddress), connectPort, par("fastOpen"));
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
        case TEST_OPEN2:
        {
            // Second, independent connection from this same app/Tcp module --
            // e.g. to prime a Fast Open cookie cache with the first connection
            // (TEST_OPEN) and then exercise it here.
            const char *localAddress = par("localAddress");
            int localPort2 = par("localPort2");
            const char *connectAddress = par("connectAddress");
            int connectPort = par("connectPort");

            socket2.bind(*localAddress ? L3Address(localAddress) : L3Address(), localPort2);
            socket2.setAutoRead(par("autoRead"));

            if (par("active2"))
                socket2.connect(L3Address(connectAddress), connectPort, par("fastOpen2"));
            else
                socket2.listenOnce();
            scheduleNextSend2();
            delete msg;
            break;
        }
        case TEST_SEND2:
            socket2.send(check_and_cast<Packet *>(msg));
            scheduleNextSend2();
            break;
        case TEST_OPEN3:
        {
            // Third, independent connection from this same app/Tcp module -- e.g.
            // to verify Fast Open blackhole-detection suppression after an earlier
            // connection (TEST_OPEN2) tripped it.
            const char *localAddress = par("localAddress");
            int localPort3 = par("localPort3");
            const char *connectAddress = par("connectAddress");
            int connectPort = par("connectPort");

            socket3.bind(*localAddress ? L3Address(localAddress) : L3Address(), localPort3);
            socket3.setAutoRead(par("autoRead"));

            if (par("active3"))
                socket3.connect(L3Address(connectAddress), connectPort, par("fastOpen3"));
            else
                socket3.listenOnce();
            scheduleNextSend3();
            delete msg;
            break;
        }
        case TEST_SEND3:
            socket3.send(check_and_cast<Packet *>(msg));
            scheduleNextSend3();
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
    if (cmd.eor)
        msg->addTagIfAbsent<TcpSendEorReq>();
    scheduleAt(cmd.tSend, msg);
}

void TcpTestClient::scheduleNextSend2()
{
    if (commands2.empty())
        return;
    Command cmd = commands2.front();
    commands2.pop_front();
    Packet *msg = new Packet(makeMsgName().c_str(), TEST_SEND2);
    const auto& bytes = makeShared<ByteCountChunk>(B(cmd.numBytes));
    msg->insertAtBack(bytes);
    scheduleAt(cmd.tSend, msg);
}

void TcpTestClient::scheduleNextSend3()
{
    if (commands3.empty())
        return;
    Command cmd = commands3.front();
    commands3.pop_front();
    Packet *msg = new Packet(makeMsgName().c_str(), TEST_SEND3);
    const auto& bytes = makeShared<ByteCountChunk>(B(cmd.numBytes));
    msg->insertAtBack(bytes);
    scheduleAt(cmd.tSend, msg);
}

void TcpTestClient::finish()
{
    EV << getFullPath() << ": received " << rcvdBytes << " bytes in " << rcvdPackets << " packets\n";
}

} // namespace inet

