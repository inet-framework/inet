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

#ifndef __TCPMAIN_H
#define __TCPMAIN_H

#include <omnetpp.h>
#include <map>


class TCPConnection;

/**
 * Implements the TCP protocol.
 *
 * This class only manages connections (TCPConnection) and dispatches
 * messages and events to them. Everything of interest takes place
 * in TCPConnection.
 */
class TCPMain : public cSimpleModule
{
  protected:
    std::map<int,TCPConnection*> tcpConns;  // connId-to-TCPConnection map
    static int nextConnId;

    TCPConnection *findConnection(int connId);
    void removeConnection(int connId, TCPConnection *conn);

  public:
    Module_Class_Members(TCPMain, cSimpleModule, 0);
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif


