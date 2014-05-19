//
// Copyright (C) 2014 OpenSim Ltd.
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// author: Zoltan Bojthe

#include <platdep/sockets.h>

#include "INETDefs.h"

#include "TCPInetListenerThread.h"

#include "ByteArrayMessage.h"
#include "ModuleAccess.h"
#include "NodeOperations.h"
#include "NodeStatus.h"
#include "SocketsRTScheduler.h"


Register_Class(TCPInetListenerThread);

TCPInetListenerThread::TCPInetListenerThread()
{
}

TCPInetListenerThread::~TCPInetListenerThread()
{
}

void TCPInetListenerThread::init(TCPMultithreadApp *module, cGate *toTcp, cMessage *msg)
{
}

void TCPInetListenerThread::processMessage(cMessage *msg)
{
    delete msg;
}

void TCPInetListenerThread::acceptInetConnection(cModule *module, cMessage *msg)
{
}

void TCPInetListenerThread::socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent)
{
}

void TCPInetListenerThread::socketEstablished(int connId, void *yourPtr)
{
}

void TCPInetListenerThread::socketPeerClosed(int connId, void *yourPtr)
{
}

void TCPInetListenerThread::socketClosed(int connId, void *yourPtr)
{
}

void TCPInetListenerThread::socketFailure(int connId, void *yourPtr, int code)
{
}

void TCPInetListenerThread::socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status)
{
    delete status;
}

