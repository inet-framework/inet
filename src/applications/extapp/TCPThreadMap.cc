//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "INETDefs.h"

#include "TCPThreadMap.h"

#include "TCPCommand_m.h"

ITCPAppThread *TCPThreadMap::findThreadFor(cMessage *msg)
{
    int connId = -1;
    if (msg->isSelfMessage())
    {
        ITCPAppThread *thread = (ITCPAppThread *)(msg->getContextPointer());
        if (thread)
            connId = thread->getConnectionId();
    }
    else
    {
        TCPCommand *ind = dynamic_cast<TCPCommand *>(msg->getControlInfo());
        if (!ind)
            throw cRuntimeError("TCPThreadMap: findSocketFor(): no TCPCommand control info in message (not from TCP?)");
        connId = ind->getConnId();
    }

    ThreadMap::iterator i = threadMap.find(connId);
    ASSERT(i==threadMap.end() || i->first==i->second->getConnectionId());
    return (i==threadMap.end()) ? NULL : i->second;
}

void TCPThreadMap::addThread(ITCPAppThread *thread)
{
    ASSERT(threadMap.find(thread->getConnectionId())==threadMap.end());
    threadMap[thread->getConnectionId()] = thread;
}

ITCPAppThread *TCPThreadMap::removeThread(ITCPAppThread *thread)
{
    ThreadMap::iterator i = threadMap.find(thread->getConnectionId());
    if (i != threadMap.end())
        threadMap.erase(i);
    return thread;
}

void TCPThreadMap::deleteThreads()
{
    for (ThreadMap::iterator i=threadMap.begin(); i != threadMap.end(); ++i)
       delete i->second;
    threadMap.clear();
}
