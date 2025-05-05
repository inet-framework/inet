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

#include "QuicOrderedReceiver.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(QuicOrderedReceiver);

void QuicOrderedReceiver::handleStartOperation(LifecycleOperation *operation)
{
    L3Address localAddress = L3AddressResolver().resolve(par("localAddress"));
    int localPort = par("localPort");

    socket.setOutputGate(gate("socketOut"));
    socket.bind(localAddress, localPort);
    socket.listen();
    EV_INFO << "listen on port " << localPort << endl;
}

void QuicOrderedReceiver::handleStopOperation(LifecycleOperation *operation)
{
    socket.close();
}

void QuicOrderedReceiver::handleCrashOperation(LifecycleOperation *operation)
{

}

void QuicOrderedReceiver::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        EV_DEBUG << "SelfMessage received" << endl;
    } else {
        switch (msg->getKind()) {
            case QUIC_I_DATA: {
                auto pkt = check_and_cast<Packet*>(msg);
                const Ptr<const Chunk> data = pkt->popAtFront();
                static simsignal_t bytesReceivedSignal = registerSignal("bytesReceived");
                emit(bytesReceivedSignal, (long)B(data->getChunkLength()).get());
                EV_DEBUG << data << " received, check..." << endl;
                checkData(data);
                break;
            }
            case QUIC_I_DATA_AVAILABLE: {
                QuicDataInfo *ctrInfo = dynamic_cast<QuicDataInfo *>(msg->getControlInfo());
                auto streamId = ctrInfo->getStreamID();
                auto avaliableDataSize = ctrInfo->getAvaliableDataSize();
                EV_DEBUG << avaliableDataSize << " bytes arrived on stream " << streamId << " - call recv" << endl;
                socket.recv(avaliableDataSize, streamId);
                break;
            }
            case QUIC_I_ESTABLISHED: {
                EV_DEBUG << "connection established" << endl;
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

void QuicOrderedReceiver::checkData(const Ptr<const Chunk> data)
{
    const Ptr<const BytesChunk> bytesChunk = dynamicPtrCast<const BytesChunk>(data);
    if (bytesChunk == nullptr) {
        EV_WARN << "Unable to check data, got " << data << endl;
        return;
    }
    for (int i=0; i<bytesChunk->getByteArraySize(); i++) {
        if (currentByte != bytesChunk->getByte(i)) {
            throw cRuntimeError("Received wrong bytes. Received byte is %d, expected byte is %d", bytesChunk->getByte(i), currentByte);
        }
        currentByte++;
    }
}

} //namespace
