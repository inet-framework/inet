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
#include "TCPInterfacePacket_m.h"
#include "IPInterfacePacket.h"


int TCPMain::nextConnId = 1;   // don't issue 0

void TCPMain::initialize()
{
    WATCH(nextConnId);
    // WATH_map(tcpConns);
}

void TCPMain::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        int connId = (int) msg->contextPointer();
        TCPConnection *conn = findConnection(connId);
        if (!conn)
        {
            ev << "event " << msg << ": corresponding connId=" << connId <<
                  " doesn't exist (any longer?), event deleted\n";
            delete msg;
            return;
        }
        bool ret = conn->processTimer(msg);
        if (!ret)
            removeConnection(connId,conn);
    }
    else if (dynamic_cast<IPInterfacePacket *>(msg))
    {
        TCPSegment *tcpseg = check_and_cast<TCPSegment *>(msg->encapsulatedMsg());
        int connId = tcpseg->connId();
        TCPConnection *conn = findConnection(connId);
        if (!conn)
        {
            if (tcpseg->synBit() && !tcpseg->ackBit())
            {
                ev << "TCP connection created for incoming SYN " << msg << ", connId=" << connId << "\n";
                conn = new TCPConnection(connId, this);
                tcpConns[connId] = conn;
            }
            else
            {
                ev << "TCP segment " << tcpseg << ": corresponding connId=" << connId <<
                      " doesn't exist (any longer?), packet dropped\n";
                // FIXME not good, maybe RST has to be sent etc
                delete tcpseg;
                return;
            }
        }
        bool ret = conn->processTCPSegment((IPInterfacePacket *)msg);
        if (!ret)
            removeConnection(connId,conn);
    }
    else if (dynamic_cast<TCPInterfacePacket *>(msg))
    {
        TCPInterfacePacket *tcpIfPacket = (TCPInterfacePacket *)msg;
        int connId = tcpIfPacket->getConnId();
        TCPConnection *conn;
        if (connId==-1)
        {
            ev << "TCP connection created for " << msg << ", connId=" << connId << "\n";
            conn = new TCPConnection(nextConnId++,this);
            tcpConns[connId] = conn;
        }
        else
        {
            conn = findConnection(connId);
            if (!conn)
            {
                error("Wrong connId=%d in TCPInterfacePacket (%s)%s, no such connection",
                      connId, msg->className(), msg->name());
            }
        }
        bool ret = conn->processAppCommand(tcpIfPacket);
        if (!ret)
            removeConnection(connId,conn);
    }
    else
    {
        error("Wrong message (%s)%s, TCPInterfacePacket or TCPSegment wrapped in IPInterfacePacket expected",
              msg->className(), msg->name());
    }

}

TCPConnection *TCPMain::findConnection(int connId)
{
    // here we actually do two lookups, but what the hell...
    if (tcpConns.find(connId)==tcpConns.end())
        return NULL;
    return tcpConns[connId];
}

void TCPMain::removeConnection(int connId, TCPConnection *conn)
{
    ev << "Deleting TCP connection connId=" << connId << "\n";
    delete conn;
    tcpConns.erase(connId);
}



