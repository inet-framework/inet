//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/applications/common/SocketTag_m.h"
#include "inet/applications/tcpapp/TcpServerSocketIo.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(TcpServerSocketIo);

void TcpServerSocketIo::acceptSocket(TcpAvailableInfo *availableInfo)
{
    Enter_Method_Silent();
    socket = new TcpSocket(availableInfo);
    socket->setOutputGate(gate("socketOut"));
    socket->setCallback(this);
    socket->accept(availableInfo->getNewSocketId());
}

void TcpServerSocketIo::handleMessage(cMessage *message)
{
    if (message->arrivedOn("socketIn")) {
        ASSERT(socket != nullptr && socket->belongsToSocket(message));
        socket->processMessage(message);
    }
    else if (message->arrivedOn("trafficIn"))
        socket->send(check_and_cast<Packet *>(message));
    else
        throw cRuntimeError("Unknown message");
}

void TcpServerSocketIo::socketDataArrived(TcpSocket* socket, Packet *packet, bool urgent)
{
    packet->removeTag<SocketInd>();
    send(packet, "trafficOut");
}

} // namespace inet

