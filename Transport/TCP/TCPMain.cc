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
#include "TCPSegment.h"
#include "TCPCommand_m.h"
#include "IPControlInfo_m.h"
#include "stlwatch.h"

Define_Module(TCPMain);


bool TCPMain::testing;
bool TCPMain::logverbose;

int TCPMain::nextConnId = 0;


static std::ostream & operator<<(std::ostream & os, const TCPMain::SockPair& sp)
{
    os << "loc=" << IPAddress(sp.localAddr) << ":" << sp.localPort << " "
       << "rem=" << IPAddress(sp.remoteAddr) << ":" << sp.remotePort;
    return os;
}

static std::ostream & operator<<(std::ostream & os, const TCPMain::AppConnKey& app)
{
    os << "connId=" << app.connId << " appGateIndex=" << app.appGateIndex;
    return os;
}

static std::ostream & operator<<(std::ostream & os, const TCPConnection& conn)
{
    os << "connId=" << conn.connId << " " << TCPConnection::stateName(conn.getFsmState())
       << " state={" << const_cast<TCPConnection&>(conn).getState()->info() << "}";
    return os;
}


void TCPMain::initialize()
{
    nextEphemeralPort = 1024;

    WATCH_PTRMAP(tcpConnMap);
    WATCH_PTRMAP(tcpAppConnMap);

    cModule *netw = simulation.systemModule();
    testing = netw->hasPar("testing") && netw->par("testing").boolValue();
    logverbose = !testing && netw->hasPar("logverbose") && netw->par("logverbose").boolValue();
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

            // add into appConnMap here; it'll be added to connMap during processing
            // the OPEN command in TCPConnection's processAppCommand().
            AppConnKey key;
            key.appGateIndex = appGateIndex;
            key.connId = connId;
            tcpAppConnMap[key] = conn;

            tcpEV << "TCP connection created for " << msg << "\n";
        }
        bool ret = conn->processAppCommand(msg);
        if (!ret)
            removeConnection(conn);
    }

    if (ev.isGUI())
        updateDisplayString();
}

void TCPMain::updateDisplayString()
{
    if (ev.disabled())
    {
        // in express mode, we don't bother to update the display
        // (std::map's iteration is not very fast if map is large)
        displayString().setTagArg("t",0,"");
        return;
    }

    //char buf[40];
    //sprintf(buf,"%d conns", tcpAppConnMap.size());
    //displayString().setTagArg("t",0,buf);

    int numINIT=0, numCLOSED=0, numLISTEN=0, numSYN_SENT=0, numSYN_RCVD=0,
        numESTABLISHED=0, numCLOSE_WAIT=0, numLAST_ACK=0, numFIN_WAIT_1=0,
        numFIN_WAIT_2=0, numCLOSING=0, numTIME_WAIT=0;

    for (TcpAppConnMap::iterator i=tcpAppConnMap.begin(); i!=tcpAppConnMap.end(); ++i)
    {
        int state = (*i).second->getFsmState();
        switch(state)
        {
           case TCP_S_INIT:        numINIT++; break;
           case TCP_S_CLOSED:      numCLOSED++; break;
           case TCP_S_LISTEN:      numLISTEN++; break;
           case TCP_S_SYN_SENT:    numSYN_SENT++; break;
           case TCP_S_SYN_RCVD:    numSYN_RCVD++; break;
           case TCP_S_ESTABLISHED: numESTABLISHED++; break;
           case TCP_S_CLOSE_WAIT:  numCLOSE_WAIT++; break;
           case TCP_S_LAST_ACK:    numLAST_ACK++; break;
           case TCP_S_FIN_WAIT_1:  numFIN_WAIT_1++; break;
           case TCP_S_FIN_WAIT_2:  numFIN_WAIT_2++; break;
           case TCP_S_CLOSING:     numCLOSING++; break;
           case TCP_S_TIME_WAIT:   numTIME_WAIT++; break;
        }
    }
    char buf2[200];
    buf2[0] = '\0';
    if (numINIT>0)       sprintf(buf2+strlen(buf2), "init:%d ", numINIT);
    if (numCLOSED>0)     sprintf(buf2+strlen(buf2), "closed:%d ", numCLOSED);
    if (numLISTEN>0)     sprintf(buf2+strlen(buf2), "listen:%d ", numLISTEN);
    if (numSYN_SENT>0)   sprintf(buf2+strlen(buf2), "syn_sent:%d ", numSYN_SENT);
    if (numSYN_RCVD>0)   sprintf(buf2+strlen(buf2), "syn_rcvd:%d ", numSYN_RCVD);
    if (numESTABLISHED>0) sprintf(buf2+strlen(buf2),"estab:%d ", numESTABLISHED);
    if (numCLOSE_WAIT>0) sprintf(buf2+strlen(buf2), "close_wait:%d ", numCLOSE_WAIT);
    if (numLAST_ACK>0)   sprintf(buf2+strlen(buf2), "last_ack:%d ", numLAST_ACK);
    if (numFIN_WAIT_1>0) sprintf(buf2+strlen(buf2), "fin_wait_1:%d ", numFIN_WAIT_1);
    if (numFIN_WAIT_2>0) sprintf(buf2+strlen(buf2), "fin_wait_2:%d ", numFIN_WAIT_2);
    if (numCLOSING>0)    sprintf(buf2+strlen(buf2), "closing:%d ", numCLOSING);
    if (numTIME_WAIT>0)  sprintf(buf2+strlen(buf2), "time_wait:%d ", numTIME_WAIT);
    displayString().setTagArg("t",0,buf2);
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

void TCPMain::addForkedConnection(TCPConnection *conn, TCPConnection *newConn, IPAddress localAddr, IPAddress remoteAddr, int localPort, int remotePort)
{
    // update conn's socket pair, and register newConn (which'll keep LISTENing)
    updateSockPair(conn, localAddr, remoteAddr, localPort, remotePort);
    updateSockPair(newConn, newConn->localAddr, newConn->remoteAddr, newConn->localPort, newConn->remotePort);

    // conn will get a new connId...
    AppConnKey key;
    key.appGateIndex = conn->appGateIndex;
    key.connId = conn->connId;
    tcpAppConnMap.erase(key);
    key.connId = conn->connId = getNewConnId();
    tcpAppConnMap[key] = conn;

    // ...and newConn will live on with the old connId
    key.appGateIndex = newConn->appGateIndex;
    key.connId = newConn->connId;
    tcpAppConnMap[key] = newConn;
}

void TCPMain::removeConnection(TCPConnection *conn)
{
    tcpEV << "Deleting TCP connection\n";

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
    tcpEV << fullPath() << ": finishing with " << tcpConnMap.size() << " connections open.\n";
}
