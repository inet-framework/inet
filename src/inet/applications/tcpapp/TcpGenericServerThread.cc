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

#include "inet/applications/tcpapp/TcpGenericServerThread.h"

#include "inet/applications/tcpapp/GenericAppMsg_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"

namespace inet {

Register_Class(TcpGenericServerThread);

void TcpGenericServerThread::established()
{
    // no initialization needed
}

void TcpGenericServerThread::dataArrived(Packet *msg, bool)
{
    const auto& appmsg = msg->peekData<GenericAppMsg>();

    if (!appmsg)
        throw cRuntimeError("Message (%s)%s is not a GenericAppMsg -- probably wrong client app",
                msg->getClassName(), msg->getName());

    if (appmsg->getReplyDelay() > 0)
        throw cRuntimeError("Cannot process (%s)%s: %s class doesn't support replyDelay field"
                            " of GenericAppMsg, try to use TcpGenericServerApp instead",
                msg->getClassName(), msg->getName(), getClassName());

    // process message: send back requested number of bytes, then close
    // connection if that was requested too
    B requestedBytes = appmsg->getExpectedReplyLength();
    bool doClose = appmsg->getServerClose();

    if (requestedBytes > B(0)) {
        Packet *outPacket = new Packet(msg->getName());
        const auto& payload = makeShared<ByteCountChunk>(requestedBytes);
        payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
        outPacket->insertAtBack(payload);
        getSocket()->send(outPacket);
    }

    if (doClose)
        getSocket()->close();
    delete msg;
}

void TcpGenericServerThread::timerExpired(cMessage *timer)
{
    // no timers in this serverThread
}

} // namespace inet

