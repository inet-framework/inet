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

#include "inet/applications/tcpapp/TcpServerListener.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

Define_Module(TcpServerListener);

void TcpServerListener::handleStartOperation(LifecycleOperation *operation)
{
    const char *localAddress = par("localAddress");
    int localPort = par("localPort");
    serverSocket.setOutputGate(gate("socketOut"));
    serverSocket.setCallback(this);
    serverSocket.bind(localAddress[0] ? L3Address(localAddress) : L3Address(), localPort);
    serverSocket.listen();
}

void TcpServerListener::handleStopOperation(LifecycleOperation *operation)
{
    for (auto connection: connectionSet)
        connection->getSocket()->close();
    serverSocket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void TcpServerListener::handleCrashOperation(LifecycleOperation *operation)
{
    while (!connectionSet.empty()) {
        auto connection = *connectionSet.begin();
        // TODO: destroy!!!
        connection->getSocket()->close();
        removeConnection(connection);
    }
    // TODO: always?
    if (operation->getRootModule() != getContainingNode(this))
        serverSocket.destroy();
}

void TcpServerListener::handleMessageWhenUp(cMessage *msg)
{
    if (serverSocket.belongsToSocket(msg))
        serverSocket.processMessage(msg);
    else
         throw cRuntimeError("Unknown incoming message: '%s'", msg->getName());
}

void TcpServerListener::finish()
{
    while (!connectionSet.empty())
        removeConnection(*connectionSet.begin());
}

void TcpServerListener::socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo)
{
    const char *serverConnectionModuleType = par("serverConnectionModuleType");
    cModuleType *moduleType = cModuleType::get(serverConnectionModuleType);
    cModule *submodule = getParentModule()->getSubmodule("connection", 0);
    int submoduleIndex = submodule == nullptr ? 0 : submodule->getVectorSize();
    auto connection = moduleType->create("connection", getParentModule(), submoduleIndex + 1, submoduleIndex);
    connection->finalizeParameters();
    connection->buildInside();
    connection->callInitialize();
    auto dispatcher = check_and_cast<cSimpleModule *>(gate("socketIn")->getPathStartGate()->getOwnerModule());
    dispatcher->setGateSize("in", dispatcher->gateSize("in") + 1);
    dispatcher->setGateSize("out", dispatcher->gateSize("out") + 1);
    connection->gate("socketOut")->connectTo(dispatcher->gate("in", dispatcher->gateSize("in") - 1));
    dispatcher->gate("out", dispatcher->gateSize("out") - 1)->connectTo(connection->gate("socketIn"));
    auto serverSocketIo = check_and_cast<TcpServerSocketIo *>(connection->gate("socketIn")->getPathEndGate()->getOwnerModule());
    serverSocketIo->acceptSocket(availableInfo);
    connectionSet.insert(serverSocketIo);
}

void TcpServerListener::socketClosed(TcpSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION && connectionSet.empty() && !serverSocket.isOpen())
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void TcpServerListener::removeConnection(TcpServerSocketIo *connection)
{
    connectionSet.erase(connection);
    connection->deleteModule();
}

void TcpServerListener::connectionClosed(TcpServerSocketIo *connection)
{
    connectionSet.erase(connection);
    socketClosed(connection->getSocket());
    connection->deleteModule();
}

} // namespace inet

