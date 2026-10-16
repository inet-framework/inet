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
#include "inet/transportlayer/contract/tcp/TcpSendEorTag_m.h"
#include "inet/transportlayer/contract/tcp/TcpSendMoreTag_m.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/transportlayer/contract/tcp/TcpTimestampingTag_m.h"

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
        bool more = false; // the SEND carries MSG_MORE (TcpSendMoreReq)
        bool eor = false; // the SEND carries MSG_EOR (TcpSendEorReq)
        bool urgent = false; // the SEND carries urgent data (TcpSendUrgentReq, MSG_OOB)
    };
    typedef std::list<Command> Commands;
    Commands commands;
    Commands readCommands; // with autoRead=false: when to READ, and how many bytes
    Commands writerBlockedCommands; // when to tell TCP that the writer is blocked (1) or not (0)
    Commands keepAliveCommands; // when to turn keepalive on (1) or off (0)
    Commands noDelayCommands; // when to turn TCP_NODELAY on (1) or off (0)
    Commands maxSegCommands; // when to set TCP_MAXSEG, and its value
    Commands pathMtuCommands; // when to set the route MTU, and its value
    Commands corkCommands; // when to turn TCP_CORK on (1) or off (0)
    Commands rcvBufCommands; // when to set SO_RCVBUF, and its value
    Commands synRetriesCommands; // when to set TCP_SYNCNT, and its value
    std::list<std::pair<std::string, std::string>> tcpParCommands; // tcpParScript: the parameters to set, in time order
    static const int NUM_EXTRA = 2; // the optional second and third connection (tOpen2, tOpen3)
    Commands extraCommands[NUM_EXTRA]; // the SEND of each extra connection

    enum { TEST_OPEN, TEST_SEND, TEST_CLOSE, TEST_ABORT, TEST_STATUS, TEST_READ, TEST_WRITER_BLOCKED, TEST_KEEPALIVE, TEST_NODELAY, TEST_MAXSEG, TEST_PATHMTU, TEST_CORK, TEST_RCVBUF, TEST_SYN_RETRIES, TEST_TCP_PAR, TEST_OPEN_EXTRA, TEST_SEND_EXTRA = TEST_OPEN_EXTRA + NUM_EXTRA };

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

        // parse an optional flag of the SEND
        while (isspace(*s)) s++;
        if (isalpha(*s)) {
            const char *w = s;
            while (isalpha(*s)) s++;
            std::string flag(w, s - w);
            if (flag == "more")
                cmd.more = true;
            else if (flag == "eor")
                cmd.eor = true;
            else if (flag == "urgent")
                cmd.urgent = true;
            else
                throw cRuntimeError("syntax error in script: unknown flag '%s'", flag.c_str());
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
    EV_INFO << "STATUS: skRcvbuf=" << status->getSkRcvbuf() << " sndbufLimited=" << status->getSndbufLimited() << "\n";
    EV_INFO << "STATUS: lastDataRecvTime=" << status->getLastDataRecvTime() << "\n";
    EV_INFO << "STATUS: rcvRtt=" << status->getRcvRtt() << " rcvSpace=" << status->getRcvSpace() << "\n";
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
    // tcpParScript: "time name value; ...", as a harness that changes a sysctl
    cStringTokenizer tcpParTokenizer(par("tcpParScript"), ";");
    while (tcpParTokenizer.hasMoreTokens()) {
        std::vector<std::string> fields = cStringTokenizer(tcpParTokenizer.nextToken()).asVector();
        if (fields.size() != 3)
            throw cRuntimeError("tcpParScript: \"time name value\" expected");
        tcpParCommands.push_back(std::make_pair(fields[1], fields[2]));
        scheduleAt(SimTime::parse(fields[0].c_str()), new cMessage("TcpPar", TEST_TCP_PAR));
    }
    parseScript(par("writerBlockedScript"), writerBlockedCommands);
    for (const auto& command : writerBlockedCommands)
        scheduleAt(command.tSend, new cMessage("WriterBlocked", TEST_WRITER_BLOCKED));
    parseScript(par("keepAliveScript"), keepAliveCommands);
    for (const auto& command : keepAliveCommands)
        scheduleAt(command.tSend, new cMessage("KeepAlive", TEST_KEEPALIVE));
    parseScript(par("noDelayScript"), noDelayCommands);
    for (const auto& command : noDelayCommands)
        scheduleAt(command.tSend, new cMessage("NoDelay", TEST_NODELAY));
    parseScript(par("maxSegScript"), maxSegCommands);
    for (const auto& command : maxSegCommands)
        scheduleAt(command.tSend, new cMessage("MaxSeg", TEST_MAXSEG));
    parseScript(par("pathMtuScript"), pathMtuCommands);
    for (const auto& command : pathMtuCommands)
        scheduleAt(command.tSend, new cMessage("PathMtu", TEST_PATHMTU));
    parseScript(par("corkScript"), corkCommands);
    for (const auto& command : corkCommands)
        scheduleAt(command.tSend, new cMessage("Cork", TEST_CORK));
    parseScript(par("rcvBufScript"), rcvBufCommands);
    for (const auto& command : rcvBufCommands)
        scheduleAt(command.tSend, new cMessage("RcvBuf", TEST_RCVBUF));
    parseScript(par("synRetriesScript"), synRetriesCommands);
    for (const auto& command : synRetriesCommands)
        scheduleAt(command.tSend, new cMessage("SynRetries", TEST_SYN_RETRIES));
    if (tClose > 0)
        scheduleAt(tClose, new cMessage("Close", TEST_CLOSE));
    simtime_t tAbort = par("tAbort");
    if (tAbort > 0)
        scheduleAt(tAbort, new cMessage("Abort", TEST_ABORT));

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
        if (par("logData")) {
            const auto& bytes = check_and_cast<Packet *>(msg)->peekDataAsBytes();
            EV_INFO << "received data:";
            for (size_t i = 0; i < std::min<size_t>(8, bytes->getBytes().size()); i++)
                EV_INFO << " " << (int)bytes->getByte(i);
            EV_INFO << "\n";
        }
    }
    else if (msg->getKind()==TCP_I_STATUS)
    {
        printStatus(check_and_cast<TcpStatusInfo *>(msg->getControlInfo()));
    }
    if (par("logIndications")) {
        EV_INFO_C("testing") << getFullName() << ": " << cEnum::get("inet::TcpStatusInd")->getStringFor(msg->getKind());
        if (auto packet = dynamic_cast<Packet *>(msg))
            EV_INFO_C("testing") << ", " << packet->getByteLength() << " bytes";
        if (auto packet = dynamic_cast<Packet *>(msg))
            if (auto timestamp = packet->findTag<TcpRxTimestampInd>())
                EV_INFO_C("testing") << ", delivered at " << timestamp->getTimestamp();
        if (auto completionInfo = dynamic_cast<TcpZerocopyCompletionInfo *>(msg->getControlInfo()))
            EV_INFO_C("testing") << ", id " << completionInfo->getZerocopyId();
        if (auto urgentInfo = dynamic_cast<TcpUrgentInfo *>(msg->getControlInfo()))
            EV_INFO_C("testing") << ", " << urgentInfo->getUrgentBytes() << " urgent bytes to read";
        if (auto packet = dynamic_cast<Packet *>(msg))
            if (auto urgentInd = packet->findTag<TcpUrgentInd>())
                EV_INFO_C("testing") << ", " << urgentInd->getRemaining() << " urgent bytes remain";
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
            socket.setAdvertisedWindow(par("socketAdvertisedWindow"));
            socket.setReceiveBufferSize(par("socketReceiveBufferSize"));
            socket.setSynRetries(par("socketSynRetries"));

            if (par("active"))
                socket.connect(L3Address(connectAddress), connectPort, par("fastOpen"));
            else if (par("fork"))
                socket.listen();
            else
                socket.listenOnce();
            if (!par("owned").boolValue())
                socket.setOwned(false);
            if (par("timestamping").boolValue())
                socket.setTimestamping(true);
            if ((int)par("notsentLowat") >= 0)
                socket.setNotsentLowat(par("notsentLowat"));
            scheduleNextSend();
            delete msg;
            break;
        }
        case TEST_SEND:
            if (par("zerocopy"))
                socket.sendZerocopy(check_and_cast<Packet *>(msg));
            else
                socket.send(check_and_cast<Packet *>(msg));
            scheduleNextSend();
            break;
        case TEST_CLOSE:
            socket.close();
            delete msg;
            break;
        case TEST_ABORT:
            socket.abort();
            delete msg;
            break;
        case TEST_STATUS:
            socket.requestStatus();
            delete msg;
            break;
        case TEST_TCP_PAR: {
            // the Tcp module at the end of the socket output gate
            cModule *tcp = gate("socketOut")->getPathEndGate()->getOwnerModule();
            auto& command = tcpParCommands.front();
            tcp->par(command.first.c_str()).parse(command.second.c_str());
            EV_INFO << "Parameter " << command.first << " of " << tcp->getFullPath() << " set to " << command.second << "\n";
            tcpParCommands.pop_front();
            delete msg;
            break;
        }
        case TEST_RCVBUF:
            socket.setReceiveBufferSize(rcvBufCommands.front().numBytes);
            rcvBufCommands.pop_front();
            delete msg;
            break;
        case TEST_SYN_RETRIES:
            socket.setSynRetries(synRetriesCommands.front().numBytes);
            synRetriesCommands.pop_front();
            delete msg;
            break;
        case TEST_CORK:
            socket.setCork(corkCommands.front().numBytes != 0);
            corkCommands.pop_front();
            delete msg;
            break;
        case TEST_MAXSEG:
            socket.setMaxSeg(maxSegCommands.front().numBytes);
            maxSegCommands.pop_front();
            delete msg;
            break;
        case TEST_PATHMTU:
            socket.setPathMtu(pathMtuCommands.front().numBytes);
            pathMtuCommands.pop_front();
            delete msg;
            break;
        case TEST_NODELAY:
            socket.setNoDelay(noDelayCommands.front().numBytes != 0);
            noDelayCommands.pop_front();
            delete msg;
            break;
        case TEST_KEEPALIVE:
            socket.setKeepAlive(keepAliveCommands.front().numBytes != 0);
            keepAliveCommands.pop_front();
            delete msg;
            break;
        case TEST_WRITER_BLOCKED:
            socket.setWriterBlocked(writerBlockedCommands.front().numBytes != 0);
            writerBlockedCommands.pop_front();
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
    if (cmd.more)
        msg->addTag<TcpSendMoreReq>();
    if (cmd.eor)
        msg->addTag<TcpSendEorReq>();
    if (cmd.urgent)
        msg->addTag<TcpSendUrgentReq>();
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

