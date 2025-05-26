//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/tcpapp/TcpServerHostApp.h"

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/stlutils.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(TcpServerHostApp);

void TcpServerHostApp::initialize(int stage)
{
    ApplicationBase::initialize(stage);
}

void TcpServerHostApp::handleStartOperation(LifecycleOperation *operation)
{
    const char *localAddress = par("localAddress");
    int localPort = par("localPort");
    bool autoRead = par("autoRead");

    serverSocket.setOutputGate(gate("socketOut"));
    serverSocket.setCallback(this);
    serverSocket.bind(localAddress[0] ? L3Address(localAddress) : L3Address(), localPort);
    serverSocket.setAutoRead(autoRead);
    serverSocket.listen();
}

void TcpServerHostApp::handleStopOperation(LifecycleOperation *operation)
{
    for (auto thread : threadSet)
        thread->close();
    serverSocket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void TcpServerHostApp::handleCrashOperation(LifecycleOperation *operation)
{
    // remove and delete threads
    while (!threadSet.empty()) {
        auto thread = *threadSet.begin();
        // TODO destroy!!!
        thread->close();
        removeThread(thread);
    }
    // TODO always?
    if (operation->getRootModule() != getContainingNode(this))
        serverSocket.destroy();
}

void TcpServerHostApp::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();

    char buf[32];
    sprintf(buf, "%d threads", socketMap.size());
    getDisplayString().setTagArg("t", 0, buf);
}

void TcpServerHostApp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        TcpServerThreadBase *thread = (TcpServerThreadBase *)msg->getContextPointer();
        if (!contains(threadSet, thread))
            throw cRuntimeError("Invalid thread pointer in the timer (msg->contextPointer is invalid)");
        thread->timerExpired(msg);
    }
    else {
        TcpSocket *socket = check_and_cast_nullable<TcpSocket *>(socketMap.findSocketFor(msg));
        if (socket)
            socket->processMessage(msg);
        else if (serverSocket.belongsToSocket(msg))
            serverSocket.processMessage(msg);
        else {
//            throw cRuntimeError("Unknown incoming message: '%s'", msg->getName());
            EV_ERROR << "message " << msg->getFullName() << "(" << msg->getClassName() << ") arrived for unknown socket \n";
            delete msg;
        }
    }
}

void TcpServerHostApp::finish()
{
    // remove and delete threads
    while (!threadSet.empty())
        removeThread(*threadSet.begin());
}

void TcpServerHostApp::socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo)
{
    // new TCP connection -- create new socket object and server process
    TcpSocket *newSocket = new TcpSocket(availableInfo);
    newSocket->setOutputGate(gate("socketOut"));

    const char *serverThreadModuleType = par("serverThreadModuleType");
    cModuleType *moduleType = cModuleType::get(serverThreadModuleType);
    char name[80];
    sprintf(name, "thread_%i", newSocket->getSocketId());
    TcpServerThreadBase *proc = check_and_cast<TcpServerThreadBase *>(moduleType->create(name, this));
    proc->finalizeParameters();
    proc->callInitialize();

    newSocket->setCallback(proc);
    proc->init(this, newSocket);

    socketMap.addSocket(newSocket);
    threadSet.insert(proc);

    socket->accept(availableInfo->getNewSocketId());
}

void TcpServerHostApp::socketClosed(TcpSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION && threadSet.empty() && !serverSocket.isOpen())
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void TcpServerHostApp::removeThread(TcpServerThreadBase *thread)
{
    Enter_Method("removeThread");
    // remove socket
    socketMap.removeSocket(thread->getSocket());
    threadSet.erase(thread);

    // remove thread object
    thread->deleteModule();
}

void TcpServerHostApp::threadClosed(TcpServerThreadBase *thread)
{
    Enter_Method("threadClosed");
    // remove socket
    socketMap.removeSocket(thread->getSocket());
    threadSet.erase(thread);

    socketClosed(thread->getSocket());

    // remove thread object
    thread->deleteModule();
}

void TcpServerThreadBase::socketDeleted(TcpSocket *socket)
{
    Enter_Method("socketDeleted");
    if (socket == sock) {
        sock = nullptr;
        hostmod->socketDeleted(socket);
    }
}

void TcpServerThreadBase::refreshDisplay() const
{
    SimpleModule::refreshDisplay();
    getDisplayString().setTagArg("t", 0, TcpSocket::stateName(sock->getState()));
}

void TcpServerThreadBase::socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent)
{
    dataArrived(msg, urgent);
}

void TcpServerThreadBase::socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo)
{
    socket->accept(availableInfo->getNewSocketId());
}

void TcpServerThreadBase::socketEstablished(TcpSocket *socket)
{
    established();
}

void TcpServerThreadBase::socketPeerClosed(TcpSocket *socket)
{
    peerClosed();
}

void TcpServerThreadBase::socketClosed(TcpSocket *socket)
{
    hostmod->threadClosed(this);
}

void TcpServerThreadBase::socketFailure(TcpSocket *socket, int code)
{
    hostmod->removeThread(this);
}

void TcpServerThreadBase::socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status)
{
    statusArrived(status);
}

TcpServerThreadBase::TcpServerThreadBase()
{
    sock = nullptr;
    hostmod = nullptr;
}

TcpServerThreadBase::~TcpServerThreadBase()
{
    delete sock;
}

void TcpServerThreadBase::init(TcpServerHostApp *hostmodule, TcpSocket *socket)
{
    hostmod = hostmodule;
    sock = socket;
}

void TcpServerThreadBase::close()
{
    omnetpp::cMethodCallContextSwitcher __ctx(hostmod);
    __ctx.methodCall("TcpSocket::close");
    sock->close();
}

} // namespace inet

