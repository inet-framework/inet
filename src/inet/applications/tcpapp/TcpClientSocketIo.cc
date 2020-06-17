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
#include "inet/applications/tcpapp/TcpClientSocketIo.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(TcpClientSocketIo);

void TcpClientSocketIo::open()
{
    socket = new TcpSocket();
    socket->setOutputGate(gate("socketOut"));
    socket->setCallback(this);
    const char *localAddress = par("localAddress");
    int localPort = par("localPort");
    socket->bind(*localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);
    const char *connectAddress = par("connectAddress");
    int connectPort = par("connectPort");
    L3Address destination;
    L3AddressResolver().tryResolve(connectAddress, destination);
    socket->connect(destination, connectPort);
}

void TcpClientSocketIo::handleMessage(cMessage *message)
{
    if (message->arrivedOn("socketIn")) {
        ASSERT(socket && socket->belongsToSocket(message));
        socket->processMessage(message);
    }
    else if (message->arrivedOn("trafficIn")) {
        if (socket == nullptr)
            open();
        socket->send(check_and_cast<Packet *>(message));
    }
    else
        throw cRuntimeError("Unknown message");
}

void TcpClientSocketIo::socketDataArrived(TcpSocket* socket, Packet *packet, bool urgent)
{
    packet->removeTag<SocketInd>();
    send(packet, "trafficOut");
}

} // namespace inet

