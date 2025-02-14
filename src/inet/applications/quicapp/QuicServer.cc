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

#include "QuicServer.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(QuicServer);

void QuicServer::handleStartOperation(LifecycleOperation *operation)
{
    L3Address localAddress = L3AddressResolver().resolve(par("localAddress"));
    int localPort = par("localPort");

    socket.setOutputGate(gate("socketOut"));
    socket.bind(localAddress, localPort);
    socket.listen();
    EV_INFO << "QuicServer::initialized listen port=" << localPort << "\n";

    timerStopRead = new cMessage("timer: app stop read data",APP_STOP_READ_DATA);

    // start timer to stop reading if stopTime is defined
    if (par("stopTime").doubleValue() > 0.0){
        scheduleAt(simTime() + par("stopTime").doubleValue(), timerStopRead);
    }
}

void QuicServer::handleStopOperation(LifecycleOperation *operation)
{
    socket.close();
}

void QuicServer::handleCrashOperation(LifecycleOperation *operation)
{

}

void QuicServer::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        EV_DEBUG << msg->getKind() << endl;
        switch (msg->getKind()) {
        case APP_START_READ_DATA:
            EV_DEBUG << "reschedule read_data timer";
            resendDataRequest(msg);
            break;
        case APP_STOP_READ_DATA:
            EV_DEBUG << "stop read_data timer";
            stopTimersReadyToRead();
            break;
        default:
            throw cRuntimeError("Invalid kind %d in self message",(int) msg->getKind());
        }
    } else {
        switch (msg->getKind()) {
        case QUIC_I_DATA:
            EV_DEBUG << "QUIC_I_DATA message received" << endl;
            readDataFromQuic(msg);
            break;
        case QUIC_I_DATA_NOTIFICATION:
            EV_DEBUG << "QUIC_I_DATA_NOTIFICATION message received" << endl;
            sendDataRequest(msg);
            break;
        case QUIC_I_ESTABLISHED:
            EV_DEBUG << "QUIC_I_ESTABLISHED message received" << endl;
            break;
        case QUIC_I_CLOSED:
            EV_DEBUG << "QUIC_I_CLOSED message received" << endl;
            break;
        case QUIC_I_ERROR:
            EV_DEBUG << "QUIC_I_ERROR message received" << endl;
            break;
        default:
            assert(false);
        }
    }
}

dataRequest *QuicServer::findOrCreateDataRequest(uint64_t streamId)
{
    auto it = dataRequestMap.find(streamId);
        if (it != dataRequestMap.end()) {
            return it->second;
        }

    dataRequest* request = new dataRequest();
    request->counter = 0;
    request->expectedDataSize = par("expectedRcvDataSize");

    request->timer = new cMessage("timer: app read data", APP_START_READ_DATA);
    dataRequestMap.insert( { streamId, request });
    return request;
}

void QuicServer::startTimerReadyToRead(uint64_t streamId)
{
    dataRequest* request = findOrCreateDataRequest(streamId);
    cMessage *msg = request->timer;

    QuicRecvCommand *ctrInfo = new QuicRecvCommand();
    ctrInfo->setStreamID(streamId);
    ctrInfo->setExpectedDataSize(request->expectedDataSize);

    msg->setControlInfo(ctrInfo);
    scheduleAt(simTime(), msg);
}

void QuicServer::sendDataRequest(cMessage *msg)
{

    QuicDataAvailableInfo *ctrInfo = dynamic_cast<QuicDataAvailableInfo *>(msg->getControlInfo());

    auto streamId = ctrInfo->getStreamID();
    auto request = findOrCreateDataRequest(streamId);
    request->avaliableDataSize = ctrInfo->getAvaliableDataSize();

    // APP send notification to quic with expectedDataSize
    QuicRecvCommand *rcvCmd = new QuicRecvCommand();
    rcvCmd->setStreamID(streamId);
    rcvCmd->setExpectedDataSize(request->expectedDataSize);

    if(request->counter == 0){
        socket.recv(rcvCmd);
        startTimerReadyToRead(streamId);
        request->counter = 1;
    }
    delete msg;
}

void QuicServer::resendDataRequest(cMessage *msg)
{
    QuicRecvCommand *ctrInfo = dynamic_cast<QuicRecvCommand *>(msg->getControlInfo());
    socket.recv(ctrInfo);

    auto request = findOrCreateDataRequest(ctrInfo->getStreamID());
    if (msg->isScheduled()) {
        cancelEvent(msg);
    }
    scheduleAt(simTime() + par("readInterval").doubleValue(), request->timer);
}

void QuicServer::readDataFromQuic(cMessage *msg)
{
    auto pkt = check_and_cast<Packet*>(msg);
    auto data = pkt->popAtFront();

    auto chunk = data.get();
    auto dataSize = chunk->getChunkLength();
    auto expDataSize = B(par("expectedRcvDataSize"));


    if(B(par("expectedRcvDataSize")) < B(dataSize)){
        if(expDataSize != B(0)){
            throw cRuntimeError("QuicServer::readDataFromQuic(): invalid dataSize");
        }
    }
    delete msg;
}

void QuicServer::stopTimersReadyToRead()
{
    auto it = dataRequestMap.begin();
    while (it != dataRequestMap.end()) {
        cancelAndDelete(it->second->timer);
        it++;
    }
    dataRequestMap.clear();
    cancelAndDelete(timerStopRead);
}

} //namespace
