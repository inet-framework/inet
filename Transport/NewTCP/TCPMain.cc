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


#include "TCPMain.h"
#include "TCPConnection.h"
#include "TCPSegment_m.h"
#include "TCPCommand_m.h"
#include "IPControlInfo_m.h"

Define_Module(TCPMain);


void TCPMain::initialize()
{
    nextEphemeralPort = 1024;

    // WATH_map(tcpConnMap);
    // WATH_map(tcpAppConnMap);
}

TCPMain::~TCPMain()
{
    while (!tcpAppConnMap.empty())
    {
        TcpAppConnMap::iterator i = tcpAppConnMap.begin();
        delete (*i).second;
        tcpAppConnMap.erase(i);
    }
}


void TCPMain::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        TCPConnection *conn = (TCPConnection *) msg->contextPointer();
        bool ret = conn->processTimer(msg);
        if (!ret)
            removeConnection(conn);
    }
    else if (msg->arrivedOn("from_ip"))
    {
        TCPSegment *tcpseg = check_and_cast<TCPSegment *>(msg);

        IPControlInfo *controlInfo = check_and_cast<IPControlInfo *>(msg->removeControlInfo());
        IPAddress srcAddr = controlInfo->srcAddr();
        IPAddress destAddr = controlInfo->destAddr();
        delete controlInfo;

        TCPConnection *conn = findConnForSegment(tcpseg, srcAddr, destAddr);
        if (!conn)
        {
            TCPConnection::segmentArrivalWhileClosed(tcpseg, srcAddr, destAddr);
            delete tcpseg;
            return;
        }
        bool ret = conn->processTCPSegment(tcpseg, srcAddr, destAddr);
        if (!ret)
            removeConnection(conn);
    }
    else // must be from app
    {
        TCPCommand *controlInfo = check_and_cast<TCPCommand *>(msg->controlInfo());
        int appGateIndex = msg->arrivalGate()->index();
        int connId = controlInfo->connId();

        TCPConnection *conn = findConnForApp(appGateIndex, connId);

        if (!conn)
        {
            conn = new TCPConnection(this,appGateIndex,connId);

            AppConnKey key;
            key.appGateIndex = appGateIndex;
            key.connId = connId;
            tcpAppConnMap[key] = conn;

            ev << "TCP connection created for " << msg << "\n";
        }
        bool ret = conn->processAppCommand(msg);
        if (!ret)
            removeConnection(conn);
    }
}

TCPConnection *TCPMain::findConnForSegment(TCPSegment *tcpseg, IPAddress srcAddr, IPAddress destAddr)
{
    SockPair key;
    key.localAddr = destAddr.getInt();
    key.remoteAddr = srcAddr.getInt();
    key.localPort = tcpseg->destPort();
    key.remotePort = tcpseg->srcPort();
    SockPair save = key;

    // try with fully qualified SockPair
    TcpConnMap::iterator i;
    i = tcpConnMap.find(key);
    if (i!=tcpConnMap.end())
        return i->second;

    // try with localAddr missing (only localPort specified in passive/active open)
    key.localAddr = 0;
    i = tcpConnMap.find(key);
    if (i!=tcpConnMap.end())
        return i->second;

    // try fully qualified local socket + blank remote socket (for incoming SYN)
    key = save;
    key.remoteAddr = 0;
    key.remotePort = -1;
    i = tcpConnMap.find(key);
    if (i!=tcpConnMap.end())
        return i->second;

    // try with blank remote socket, and localAddr missing (for incoming SYN)
    key.localAddr = 0;
    i = tcpConnMap.find(key);
    if (i!=tcpConnMap.end())
        return i->second;

    // given up
    return NULL;
}

TCPConnection *TCPMain::findConnForApp(int appGateIndex, int connId)
{
    AppConnKey key;
    key.appGateIndex = appGateIndex;
    key.connId = connId;

    TcpAppConnMap::iterator i = tcpAppConnMap.find(key);
    return i==tcpAppConnMap.end() ? NULL : i->second;
}

short TCPMain::getEphemeralPort()
{
    if (nextEphemeralPort==5000)
        error("Ephemeral port range 1024..4999 exhausted (email TCP model "
              "author that he should implement reuse of ephemeral ports!!!)");
    return nextEphemeralPort++;
}

void TCPMain::updateSockPair(TCPConnection *conn, IPAddress localAddr, IPAddress remoteAddr, int localPort, int remotePort)
{
    SockPair key;
    key.localAddr = conn->localAddr.getInt();
    key.remoteAddr = conn->remoteAddr.getInt();
    key.localPort = conn->localPort;
    key.remotePort = conn->remotePort;
    TcpConnMap::iterator i = tcpConnMap.find(key);
    if (i!=tcpConnMap.end())
    {
        ASSERT(i->second==conn);
        tcpConnMap.erase(i);
    }

    key.localAddr = (conn->localAddr = localAddr).getInt();
    key.remoteAddr = (conn->remoteAddr = remoteAddr).getInt();
    key.localPort = conn->localPort = localPort;
    key.remotePort = conn->remotePort = remotePort;
    tcpConnMap[key] = conn;
}

void TCPMain::removeConnection(TCPConnection *conn)
{
    ev << "Deleting TCP connection\n";

    AppConnKey key;
    key.appGateIndex = conn->appGateIndex;
    key.connId = conn->connId;
    tcpAppConnMap.erase(key);

    SockPair key2;
    key2.localAddr = conn->localAddr.getInt();
    key2.remoteAddr = conn->remoteAddr.getInt();
    key2.localPort = conn->localPort;
    key2.remotePort = conn->remotePort;
    tcpConnMap.erase(key2);

    delete conn;
}

void TCPMain::finish()
{
    ev << fullPath() << ": finishing with " << tcpConnMap.size() << " connections open.\n";
}
