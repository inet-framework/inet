//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
// Copyright (C) 2015 Thomas Dreibholz (dreibh@simula.no)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
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

#include "inet/applications/httptools/server/HttpServer.h"

namespace inet {

namespace httptools {

Define_Module(HttpServer);

void HttpServer::initialize(int stage)
{
    HttpServerBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        numBroken = 0;
        socketsOpened = 0;

        WATCH(numBroken);
        WATCH(socketsOpened);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        EV_DEBUG << "Initializing server component (sockets version)" << endl;

        int port = par("port");

        if (!useSCTP) {
            TCPSocket tcpListenSocket;
            tcpListenSocket.setOutputGate(gate("tcpOut"));
            tcpListenSocket.setDataTransferMode(TCP_TRANSFER_OBJECT);
            tcpListenSocket.bind(port);
            tcpListenSocket.setCallbackObject(this);
            tcpListenSocket.listen();
        }
        else {
            SCTPSocket sctpListenSocket;
            sctpListenSocket.setOutputGate(gate("sctpOut"));
            sctpListenSocket.bind(port);
            sctpListenSocket.setCallbackObject(this);
            sctpListenSocket.listen(true);
        }
    }
}

void HttpServer::finish()
{
    HttpServerBase::finish();

    EV_INFO << "Sockets opened: " << socketsOpened << endl;
    EV_INFO << "Broken connections: " << numBroken << endl;

    recordScalar("sock.opened", socketsOpened);
    recordScalar("sock.broken", numBroken);

    // Clean up sockets and data structures
    tcpSockCollection.deleteSockets();
    sctpSockCollection.deleteSockets();
}

void HttpServer::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        // Self messages not used at the moment
    }
    else {
        EV_DEBUG << "Handle inbound message " << msg->getName() << " of kind " << msg->getKind() << endl;
        if (!useSCTP) {
            TCPSocket *tcpSocket = tcpSockCollection.findSocketFor(msg);
            if (!tcpSocket) {
                EV_DEBUG << "No socket found for the message. Create a new one" << endl;
                // new connection -- create new socket object and server process
                tcpSocket = new TCPSocket(msg);
                tcpSocket->setOutputGate(gate("tcpOut"));
                tcpSocket->setDataTransferMode(TCP_TRANSFER_OBJECT);
                tcpSocket->setCallbackObject(this, tcpSocket);
                tcpSockCollection.addSocket(tcpSocket);
            }
            EV_DEBUG << "Process the message " << msg->getName() << endl;
            tcpSocket->processMessage(msg);
        }
        else {
            SCTPSocket *sctpSocket = sctpSockCollection.findSocketFor(msg);
            if (!sctpSocket) {
                sctpSocket = new SCTPSocket(msg);
                sctpSocket->setOutputGate(gate("sctpOut"));
                sctpSocket->setCallbackObject(this, sctpSocket);
                sctpSockCollection.addSocket(sctpSocket);
            }
            EV_DEBUG << "Process the message " << msg->getName() << endl;
            sctpSocket->processMessage(msg);
        }
    }
    updateDisplay();
}

void HttpServer::socketEstablished(int connId, void *yourPtr)
{
    EV_INFO << "connected socket with id=" << connId << endl;
    socketsOpened++;
}

void HttpServer::socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent)
{
    if (yourPtr == nullptr) {
        EV_ERROR << "Socket establish failure. Null pointer" << endl;
        return;
    }

    // Should be a HttpReplyMessage
    EV_DEBUG << "Socket data arrived on connection " << connId << ". Message=" << msg->getName() << ", kind=" << msg->getKind() << endl;

    // call the message handler to process the message.
    SCTPSimpleMessage *reply = handleReceivedMessage(msg);
    if (reply != nullptr) {
        if (!useSCTP) {
            TCPSocket *tcpSocket = (TCPSocket *)yourPtr;
            tcpSocket->send(reply);     // Send to socket if the reply is non-zero.
        }
        else {
            SCTPSocket *sctpSocket = (SCTPSocket *)yourPtr;

            const long totalReplyLength = reply->getByteLength();
            if (totalReplyLength > maxMsgSize) {
                long toSend = totalReplyLength;
                while (toSend > maxMsgSize) {
                    const long txLength = std::min(toSend, maxMsgSize);

                    HttpFragmentMessage* dummyMsg = new HttpFragmentMessage;
                    dummyMsg->setByteLength(txLength);
                    dummyMsg->setDataArraySize(dummyMsg->getByteLength());
                    for (int i = 0; i < dummyMsg->getByteLength(); i++) {
                        dummyMsg->setData(i, 'd');   // dummy data
                    }
                    dummyMsg->setDataLen(dummyMsg->getByteLength());
                    sctpSocket->send(dummyMsg);

                    toSend -= txLength;
                }
                reply->setByteLength(toSend);
                reply->setDataLen(toSend);
            }

            sctpSocket->send(reply);    // Send to socket if the reply is non-zero.
        }
    }
    delete msg;    // Delete the received message here. Must not be deleted in the handler!
}

void HttpServer::socketDataNotificationArrived(int assocId, void *yourPtr, cPacket *msg)
{
    // SCTP data is available => tell SCTP to forward it!
    const SCTPCommand* dataIndication = check_and_cast<const SCTPCommand*>(msg->getControlInfo());

    SCTPSendCommand* command = new SCTPSendCommand("SendCommand");
    command->setAssocId(dataIndication->getAssocId());
    command->setSid(dataIndication->getSid());
    command->setNumMsgs(dataIndication->getNumMsgs());

    cPacket* cmsg = new cPacket("SCTP_C_RECEIVE");
    cmsg->setKind(SCTP_C_RECEIVE);
    cmsg->setControlInfo(command);
    send(cmsg, "sctpOut");
}

void HttpServer::socketPeerClosed(int connId, void *yourPtr)
{
    if (yourPtr == nullptr) {
        EV_ERROR << "Socket establish failure. Null pointer" << endl;
        return;
    }

    // close the connection (if not already closed)
    if (!useSCTP) {
        TCPSocket *tcpSocket = (TCPSocket *)yourPtr;
        if (tcpSocket->getState() == TCPSocket::PEER_CLOSED) {
            EV_INFO << "remote TCP closed, closing here as well. Connection id is " << connId << endl;
            tcpSocket->close();    // Call the close method to properly dispose of the socket.
        }
    }
    else {
        SCTPSocket *sctpSocket = (SCTPSocket *)yourPtr;
        if (sctpSocket->getState() == SCTPSocket::PEER_CLOSED) {
            EV_INFO << "remote SCTP closed, closing here as well. Connection id is " << connId << endl;
            sctpSocket->close();    // Call the close method to properly dispose of the socket.
        }
    }
}

void HttpServer::socketClosed(int connId, void *yourPtr)
{
    EV_INFO << "connection closed. Connection id " << connId << endl;
    if (yourPtr == nullptr) {
        EV_ERROR << "Socket establish failure. Null pointer" << endl;
        return;
    }

    // Cleanup
    if (!useSCTP) {
        TCPSocket *tcpSocket = (TCPSocket *)yourPtr;
        tcpSockCollection.removeSocket(tcpSocket);
        delete tcpSocket;
    }
    else {
        SCTPSocket *sctpSocket = (SCTPSocket *)yourPtr;
        sctpSockCollection.removeSocket(sctpSocket);
        delete sctpSocket;
   }
}

void HttpServer::socketFailure(int connId, void *yourPtr, int code)
{
    EV_WARN << "connection broken. Connection id " << connId << endl;
    numBroken++;
    EV_INFO << "connection closed. Connection id " << connId << endl;
    if (yourPtr == nullptr) {
        EV_ERROR << "Socket establish failure. Null pointer" << endl;
        return;
    }

    // Cleanup
    if (!useSCTP) {
        TCPSocket *tcpSocket = (TCPSocket *)yourPtr;
        if (code == TCP_I_CONNECTION_RESET) {
            EV_WARN << "Connection reset!\n";
        }
        else if (code == TCP_I_CONNECTION_REFUSED) {
            EV_WARN << "Connection refused!\n";
        }
        tcpSockCollection.removeSocket(tcpSocket);
        delete tcpSocket;
    }
    else {
        SCTPSocket *sctpSocket = (SCTPSocket *)yourPtr;
        if (code == SCTP_I_CONNECTION_RESET) {
            EV_WARN << "Connection reset!\n";
        }
        else if (code == SCTP_I_CONNECTION_REFUSED) {
            EV_WARN << "Connection refused!\n";
        }
        sctpSockCollection.removeSocket(sctpSocket);
        delete sctpSocket;
   }
}

} // namespace httptools

} // namespace inet
