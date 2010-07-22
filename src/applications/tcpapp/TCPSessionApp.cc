//
// Copyright 2004 Andras Varga
//
// This library is free software, you can redistribute it and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


#include "TCPSessionApp.h"
#include "IPAddressResolver.h"


Define_Module(TCPSessionApp);


void TCPSessionApp::parseScript(const char *script)
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

void TCPSessionApp::count(cMessage *msg)
{
    if(msg->isPacket())
    {
        if (msg->getKind()==TCP_I_DATA || msg->getKind()==TCP_I_URGENT_DATA)
        {
            packetsRcvd++;
            bytesRcvd+=PK(msg)->getByteLength();
        }
        else
        {
            EV << "TCPSessionApp received unknown message (kind:" << msg->getKind() << ", name:" << msg->getName() << ")\n";
        }
    }
    else
    {
        indicationsRcvd++;
    }
}

void TCPSessionApp::waitUntil(simtime_t t)
{
    if (simTime()>=t)
        return;

    cMessage *timeoutMsg = new cMessage("timeout");
    scheduleAt(t, timeoutMsg);
    cMessage *msg=NULL;
    while ((msg=receive())!=timeoutMsg)
    {
        count(msg);
        socket.processMessage(msg);
    }
    delete timeoutMsg;
}

void TCPSessionApp::activity()
{
    packetsRcvd = indicationsRcvd = 0;
    bytesRcvd = bytesSent = 0;
    WATCH(packetsRcvd);
    WATCH(bytesRcvd);
    WATCH(indicationsRcvd);

    // parameters
    const char *address = par("address");
    int port = par("port");
    const char *connectAddress = par("connectAddress");
    int connectPort = par("connectPort");

    bool active = par("active");
    simtime_t tOpen = par("tOpen");
    simtime_t tSend = par("tSend");
    long sendBytes = par("sendBytes");
    simtime_t tClose = par("tClose");

    const char *script = par("sendScript");
    parseScript(script);
    if (sendBytes>0 && commands.size()>0)
        throw cRuntimeError("cannot use both sendScript and tSend+sendBytes");

    socket.setOutputGate(gate("tcpOut"));

    // open
    waitUntil(tOpen);

    socket.bind(*address ? IPvXAddress(address) : IPvXAddress(), port);

    EV << "issuing OPEN command\n";
    if (ev.isGUI()) getDisplayString().setTagArg("t",0, active?"connecting":"listening");

    if (active)
        socket.connect(IPAddressResolver().resolve(connectAddress), connectPort);
    else
        socket.listenOnce();

    // wait until connection gets established
    while (socket.getState()!=TCPSocket::CONNECTED)
    {
        socket.processMessage(receive());
        if (socket.getState()==TCPSocket::SOCKERROR)
            return;
    }

    EV << "connection established, starting sending\n";
    if (ev.isGUI()) getDisplayString().setTagArg("t",0,"connected");

    // send
    if (sendBytes>0)
    {
        waitUntil(tSend);
        EV << "sending " << sendBytes << " bytes\n";
        cPacket *msg = new cPacket("data1");
        msg->setByteLength(sendBytes);
        bytesSent += sendBytes;
        socket.send(msg);
    }
    for (CommandVector::iterator i=commands.begin(); i!=commands.end(); ++i)
    {
        waitUntil(i->tSend);
        EV << "sending " << i->numBytes << " bytes\n";
        cPacket *msg = new cPacket("data1");
        msg->setByteLength(i->numBytes);
        bytesSent += i->numBytes;
        socket.send(msg);
    }

    // close
    if (tClose>=0)
    {
        waitUntil(tClose);
        EV << "issuing CLOSE command\n";
        if (ev.isGUI()) getDisplayString().setTagArg("t",0,"closing");
        socket.close();
    }

    // wait until peer closes too and all data arrive
    for (;;)
    {
        cMessage *msg = receive();
        count(msg);
        socket.processMessage(msg);
    }
}

void TCPSessionApp::finish()
{
    EV << getFullPath() << ": received " << bytesRcvd << " bytes in " << packetsRcvd << " packets\n";
    recordScalar("bytesRcvd", bytesRcvd);
    recordScalar("bytesSent", bytesSent);
}
