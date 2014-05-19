//
// Copyright (C) 2014 OpenSim Ltd.
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

#ifndef __INET_TCPTHREAD
#define __INET_TCPTHREAD


#include "INETDefs.h"

class TCPMultithreadApp;

class INET_API ITCPAppThread
{
  public:
    virtual ~ITCPAppThread();
    virtual void init(TCPMultithreadApp *module, cGate *toTcp, cMessage *msg) = 0;
    virtual void processMessage(cMessage *msg) = 0;
    virtual int getConnectionId() const = 0;
};

class INET_API ITCPMultithreadApp
{
  public:
    virtual ~ITCPMultithreadApp();
    virtual void addThread(ITCPAppThread *thread) = 0;
    virtual void removeThread(ITCPAppThread *thread) = 0;
};

#endif

