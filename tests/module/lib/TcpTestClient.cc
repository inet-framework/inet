//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <vector>
#include <string>

#include "inet/common/INETDefs.h"

#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
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
    Commands readCommands; // with autoRead=false: when to READ, and how many bytes
    static const int NUM_EXTRA = 2; // the optional second and third connection (tOpen2, tOpen3)
    Commands extraCommands[NUM_EXTRA]; // the SEND of each extra connection

    enum { TEST_OPEN, TEST_SEND, TEST_CLOSE, TEST_STATUS, TEST_READ, TEST_OPEN_EXTRA, TEST_SEND_EXTRA = TEST_OPEN_EXTRA + NUM_EXTRA };

    int ctr;

    TcpSocket socket;
    TcpSocket extraSockets[NUM_EXTRA];

    // statistics
    int64_t rcvdBytes;
    int rcvdPackets;

  protected:
    void parseScript(const char *script, Commands& commands);
    void parseStatusRequestScript(const char *script);
    void printStatus(TcpStatusInfo *status);
    std::string makeMsgName();
    void handleSelfMessage(cMessage *msg);
    void scheduleNextSend();
    void scheduleNextExtraSend(int i);
    cPar& extraPar(const char *name, int i) { return par((name + std::to_string(i + 2)).c_str()); }

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

Define_Module(TcpTestClient);


void TcpTestClient::parseScript(const char *script, Commands& commands)
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

void TcpTestClient::parseStatusRequestScript(const char *script)
{
    const char *s = script;
    while (*s) {
        while (isspace(*s)) s++;
        if (!*s) break;
        const char *s0 = s;
        simtime_t t = strtod(s, &const_cast<char *&>(s));
        if (s == s0)
            throw cRuntimeError("syntax error in statusRequestScript: simulation time expected");
        scheduleAt(t, new cMessage("StatusRequest", TEST_STATUS));
        while (isspace(*s)) s++;
        if (!*s) break;
        if (*s != ',')
            throw cRuntimeError("syntax error in statusRequestScript: separator ',' missing");
        s++;
    }
}

void TcpTestClient::printStatus(TcpStatusInfo *status)
{
    EV_INFO << "STATUS: caState=" << status->getCaState()
            << " backoff=" << status->getBackoff()
            << " lost=" << status->getLost()
            << " probes=" << status->getProbes()
            << " bytesReceived=" << status->getBytesReceived()
            << " busyTime=" << status->getBusyTime()
            << " rwndLimited=" << status->getRwndLimited()
            << " deliveredBytes=" << status->getDeliveredBytes()
            << "\n";
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

    parseScript(script, commands);
    if (cmd.numBytes > 0 && commands.size() > 1)
        throw cRuntimeError("cannot use both sendScript and tSend+sendBytes");

    socket.setOutputGate(gate("socketOut"));
    for (auto& extraSocket : extraSockets)
        extraSocket.setOutputGate(gate("socketOut"));

    ctr = 0;

    scheduleAt(tOpen, new cMessage("Open", TEST_OPEN));
    parseStatusRequestScript(par("statusRequestScript"));
    parseScript(par("readScript"), readCommands);
    for (const auto& read : readCommands)
        scheduleAt(read.tSend, new cMessage("Read", TEST_READ));
    if (tClose > 0)
        scheduleAt(tClose, new cMessage("Close", TEST_CLOSE));

    for (int i = 0; i < NUM_EXTRA; i++) {
        simtime_t tOpenExtra = extraPar("tOpen", i);
        if (tOpenExtra >= SIMTIME_ZERO) {
            Command extraCmd;
            extraCmd.tSend = extraPar("tSend", i);
            extraCmd.numBytes = extraPar("sendBytes", i);
            if (extraCmd.numBytes > 0)
                extraCommands[i].push_back(extraCmd);
            scheduleAt(tOpenExtra, new cMessage(("Open" + std::to_string(i + 2)).c_str(), TEST_OPEN_EXTRA + i));
        }
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
    else if (msg->getKind()==TCP_I_STATUS)
    {
        printStatus(check_and_cast<TcpStatusInfo *>(msg->getControlInfo()));
    }
    if (par("logIndications")) {
        EV_INFO_C("testing") << getFullName() << ": " << cEnum::get("inet::TcpStatusInd")->getStringFor(msg->getKind());
        if (auto packet = dynamic_cast<Packet *>(msg))
            EV_INFO_C("testing") << ", " << packet->getByteLength() << " bytes";
        EV_INFO_C("testing") << "\n";
    }
    for (auto& extraSocket : extraSockets) {
        if (extraSocket.belongsToSocket(msg)) {
            extraSocket.processMessage(msg);
            return;
        }
    }
    if (socket.belongsToSocket(msg))
        socket.processMessage(msg);
    else
        delete msg; // a connection that the listener forked (fork=true): the received bytes are counted above
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
            else if (par("fork"))
                socket.listen();
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
        case TEST_STATUS:
            socket.requestStatus();
            delete msg;
            break;
        case TEST_READ:
            socket.read(readCommands.front().numBytes);
            readCommands.pop_front();
            delete msg;
            break;
        default:
            if (msg->getKind() >= TEST_OPEN_EXTRA && msg->getKind() < TEST_OPEN_EXTRA + NUM_EXTRA) {
                int i = msg->getKind() - TEST_OPEN_EXTRA;
                const char *localAddress = par("localAddress");
                const char *connectAddress = par("connectAddress");
                extraSockets[i].bind(*localAddress ? L3Address(localAddress) : L3Address(), extraPar("localPort", i));
                extraSockets[i].setAutoRead(par("autoRead"));
                if (extraPar("active", i))
                    extraSockets[i].connect(L3Address(connectAddress), par("connectPort"), extraPar("fastOpen", i));
                else
                    extraSockets[i].listenOnce();
                scheduleNextExtraSend(i);
                delete msg;
            }
            else if (msg->getKind() >= TEST_SEND_EXTRA && msg->getKind() < TEST_SEND_EXTRA + NUM_EXTRA) {
                int i = msg->getKind() - TEST_SEND_EXTRA;
                extraSockets[i].send(check_and_cast<Packet *>(msg));
                scheduleNextExtraSend(i);
            }
            else
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
    if (cmd.numBytes > 0) // a SEND of 0 bytes is a packet without content
        msg->insertAtBack(makeShared<ByteCountChunk>(B(cmd.numBytes)));
    scheduleAt(cmd.tSend, msg);
}

void TcpTestClient::scheduleNextExtraSend(int i)
{
    if (extraCommands[i].empty())
        return;
    Command cmd = extraCommands[i].front();
    extraCommands[i].pop_front();
    Packet *msg = new Packet(makeMsgName().c_str(), TEST_SEND_EXTRA + i);
    msg->insertAtBack(makeShared<ByteCountChunk>(B(cmd.numBytes)));
    scheduleAt(cmd.tSend, msg);
}

void TcpTestClient::finish()
{
    EV << getFullPath() << ": received " << rcvdBytes << " bytes in " << rcvdPackets << " packets\n";
}

} // namespace inet

