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

#include "QuicDiscardServer.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(QuicDiscardServer);

void QuicDiscardServer::handleStartOperation(LifecycleOperation *operation)
{
    L3Address localAddress = L3AddressResolver().resolve(par("localAddress"));
    int localPort = par("localPort");

    listeningSocket.setOutputGate(gate("socketOut"));
    listeningSocket.bind(localAddress, localPort);
    listeningSocket.listen();
    EV_INFO << "listen on port " << localPort << endl;
}

void QuicDiscardServer::handleStopOperation(LifecycleOperation *operation)
{
    if (clientSocket == nullptr) {
        listeningSocket.close();
    } else {
        clientSocket->close();
    }
}

void QuicDiscardServer::handleCrashOperation(LifecycleOperation *operation)
{

}

void QuicDiscardServer::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        EV_DEBUG << "SelfMessage received" << endl;
    } else {
        switch (msg->getKind()) {
            case QUIC_I_DATA: {
                auto pkt = check_and_cast<Packet*>(msg);
                auto data = pkt->popAtFront();
                static simsignal_t bytesReceivedSignal = registerSignal("bytesReceived");
                emit(bytesReceivedSignal, (long)B(data->getChunkLength()).get());
                EV_DEBUG << data << " received and discarded" << endl;
                break;
            }
            case QUIC_I_DATA_NOTIFICATION: {
                QuicDataAvailableInfo *ctrInfo = dynamic_cast<QuicDataAvailableInfo *>(msg->getControlInfo());
                auto streamId = ctrInfo->getStreamID();
                auto avaliableDataSize = ctrInfo->getAvaliableDataSize();
                EV_DEBUG << avaliableDataSize << " bytes arrived on stream " << streamId << " - call recv" << endl;
                clientSocket->recv(avaliableDataSize, streamId);
                break;
            }
            case QUIC_I_ESTABLISHED: {
                EV_DEBUG << "connection established" << endl;
                break;
            }
            case QUIC_I_AVAILABLE: {
                EV_DEBUG << "connection available" << endl;
                clientSocket = listeningSocket.accept();
                listeningSocket.close();
                break;
            }
            case QUIC_I_CLOSED: {
                EV_DEBUG << "connection closed" << endl;
                break;
            }
            case QUIC_I_ERROR: {
                EV_DEBUG << "Got QUIC error" << endl;
                break;
            }
            default: {
                assert(false);
            }
        }
    }
    delete msg;
}

} //namespace
