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

#include "TCPExtActiveThread.h"


Register_Class(TCPExtActiveThread);

TCPExtActiveThread::TCPExtActiveThread()
{
    extSocketId = INVALID_SOCKET;
    appModule = NULL;
    rtScheduler = NULL;
}

TCPExtActiveThread::~TCPExtActiveThread()
{
}

void TCPExtActiveThread::init(TCPMultithreadApp *module, cGate *toTcp, cMessage *msg)
{
    if (extSocketId != INVALID_SOCKET || appModule || rtScheduler)
        throw cRuntimeError("TCPExtActiveThread: Model error: socket already used");
    appModule = module;
    extSocketId = msg->par("newfd").longValue();
    rtScheduler->addSocket(appModule, this, extSocketId, false);
}

void TCPExtActiveThread::processMessage(cMessage *msg)
{
    switch(msg->getKind())
    {
        case SocketsRTScheduler::DATA:
        {
            if (extSocketId == INVALID_SOCKET)
                throw cRuntimeError("TCPExtActiveThread: socket not opened");
            if (extSocketId != msg->par("fd").longValue())
                throw cRuntimeError("TCPExtActiveThread: unknown socket id");
            ByteArrayMessage *pk = check_and_cast<ByteArrayMessage *>(msg);
            inetSocket.send(pk);
            break;
        }
        case SocketsRTScheduler::CLOSED:
            if (extSocketId != msg->par("fd").longValue())
                throw cRuntimeError("TCPExtActiveThread: unknown socket id");
            closesocket(extSocketId);
            extSocketId = INVALID_SOCKET;
            delete msg;
            break;
        case SocketsRTScheduler::ACCEPT:
            throw cRuntimeError("TCPExtActiveThread: socket already opened");
            delete msg;
            break;
    }
}

void TCPExtActiveThread::socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent)
{
    ByteArray& bytes = check_and_cast<ByteArrayMessage *>(msg)->getByteArray();
    ::send(extSocketId, bytes.getDataArrayPointer(), bytes.getDataArraySize(), 0);
}

void TCPExtActiveThread::socketEstablished(int connId, void *yourPtr)
{
    //TODO csak itt kellene az ACCEPT az extSocket felee
}

void TCPExtActiveThread::closeExtSocket()
{
    if (extSocketId != INVALID_SOCKET)
    {
        closesocket(extSocketId);
        rtScheduler->removeSocket(appModule, extSocketId);
        extSocketId = INVALID_SOCKET;
    }
}

void TCPExtActiveThread::socketPeerClosed(int connId, void *yourPtr)
{
    if (inetSocket.getState() == TCPSocket::PEER_CLOSED)
    {
        EV_INFO << "remote TCP closed, closing here as well\n";
        inetSocket.close();
    }
    closeExtSocket();
}

void TCPExtActiveThread::socketClosed(int connId, void *yourPtr)
{
    closeExtSocket();
}

void TCPExtActiveThread::socketFailure(int connId, void *yourPtr, int code)
{
    closeExtSocket();
}

