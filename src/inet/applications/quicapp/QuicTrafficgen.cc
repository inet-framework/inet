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

#include "QuicTrafficgen.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"

namespace inet {

Define_Module(QuicTrafficgen);

QuicTrafficgen::QuicTrafficgen() {
    timerConnect = new cMessage("QuicTrafficgen Timer - Connect");
    timerConnect->setKind(TIMER_CONNECT);

    timerLimitRuntime = new cMessage("QuicTrafficgen Timer - Runtime limit");
    timerLimitRuntime->setKind(TIMER_LIMIT_RUNTIME);

    generatorsActive = 0;
}

QuicTrafficgen::~QuicTrafficgen() {
    cancelAndDelete(timerConnect);
    cancelAndDelete(timerLimitRuntime);
}

void QuicTrafficgen::handleStartOperation(LifecycleOperation *operation)
{
    EV_DEBUG << "initialize QuicTrafficgen" << endl;

    connectPort = par("connectPort");
    connectAddress = L3AddressResolver().resolve(par("connectAddress"));
    socket.setOutputGate(gate("socketOut"));
    socket.setCallback(this);

    L3Address localAddress = L3AddressResolver().resolve(par("localAddress"));
    int localPort = par("localPort");
    socket.bind(localAddress, localPort);

    scheduleAt(par("connectTime"), timerConnect);
}

void QuicTrafficgen::handleStopOperation(LifecycleOperation *operation)
{
    EV_INFO << "handleStopOperation" << endl;
    cancelEvent(timerConnect);
    cancelEvent(timerLimitRuntime);
}

void QuicTrafficgen::handleCrashOperation(LifecycleOperation *operation)
{
}

void QuicTrafficgen::handleMessageWhenUp(cMessage *msg)
{
    EV_DEBUG << "handle message of kind " << msg->getKind() << endl;
    if (msg->isSelfMessage()) {
        handleTimeout(msg);
    } else if (msg->arrivedOn("generatorIn")) { // from generator
        handleMessageFromGenerator(msg);
        delete msg;
    } else if (msg->arrivedOn("socketIn")) { // from QUIC
        // TODO: Add and handle events: case QUIC_I_SENDQUEUE_DRAINING and QUIC_I_SENDQUEUE_FULL
        socket.processMessage(msg);
        //delete msg;
    } else { // something really strange...
        throw cRuntimeError("Invalid message: %d", (int) msg->getKind());
    }
}

void QuicTrafficgen::handleTimeout(cMessage *msg)
{
    EV_DETAIL << "handle timeout of kind " << msg->getKind() << endl;
    switch (msg->getKind()) {
        case TIMER_CONNECT:
            setStatusString("connecting");
            EV_INFO << "connect - address: " << connectAddress << endl;

            //socket.connect(connectAddress, connectPort, 0, 0, 0);
            socket.connect(connectAddress, connectPort);
            break;
        case TIMER_LIMIT_RUNTIME:
            socket.close();
            finish();
            break;
        default:
            throw cRuntimeError("Invalid timer: %d", (int) msg->getKind());
    }
}

void QuicTrafficgen::handleMessageFromGenerator(cMessage *msg)
{
    EV_DETAIL << "handle message from Generator of kind " << msg->getKind() << endl;
    switch (msg->getKind()) {
        case TRAFFICGEN_MSG_INFO:
            handleGeneratorInfo(check_and_cast<TrafficgenInfo*>(msg));
            break;
        case TRAFFICGEN_MSG_DATA:
            if (sendingAllowed) {
                sendData(check_and_cast<TrafficgenData*>(msg));
            } else {
                sendGeneratorControl(TRAFFICGEN_STOP_SENDING);
                EV_INFO << "Sending not allowed but generator triggered message" << endl;
            }
            break;
        default:
            throw cRuntimeError("Invalid message from generator: %d", (int) msg->getKind());
    }
}

void QuicTrafficgen::handleGeneratorInfo(TrafficgenInfo* msg) {
    switch(msg->getType()) {
        case TRAFFICGEN_INFO_INIT:
            generatorsActive++;
            EV_DETAIL << "New generator : " << msg->getId() << " - priority: " << msg->getPriority() << endl;

            if (streams.find(msg->getId()) != streams.end()) {
                throw cRuntimeError("Stream %d already registered", (int) msg->getId());
            }

            Stream stream;
            stream.finished = false;
            stream.msgCount = 0;
            stream.priority = msg->getPriority();
            stream.streamId = getStreamId(msg->getId()); // esv check if it is needed!
            streams.insert(std::make_pair(msg->getId(), stream));
            break;

        case TRAFFICGEN_INFO_FINISH:
            generatorsActive--;
            if (generatorsActive == 0) {
                cancelEvent(timerLimitRuntime);
                close();
            }
            break;

        default:
            throw cRuntimeError("Invalid info message from generator: %d", (int) msg->getType());
    }
}

void QuicTrafficgen::socketEstablished(QuicSocket *socket) {
    EV_INFO << "socketEstablished" << endl;
    sendGeneratorControl(TRAFFICGEN_START_SENDING);
    sendingAllowed = true;
    setStatusString("connected");
}

void QuicTrafficgen::socketDataArrived(QuicSocket* socket, Packet *packet) {
    EV_DEBUG << "Data arrived" << endl;
}

void QuicTrafficgen::socketClosed(QuicSocket *socket) {
    EV_INFO << "socketClosed" << endl;
    setStatusString("closed");
}

void QuicTrafficgen::sendGeneratorControl(uint8_t controlMessageType) {

    TrafficgenControl* ctrl = new TrafficgenControl("TrafficgenControl");
    ctrl->setKind(TRAFFICGEN_MSG_CONTROL);
    ctrl->setControlMessageType(controlMessageType);

    int outGateBaseId = gateBaseId("generatorOut");
    for (uint32_t i = 0; i < streams.size(); i++) {
        send(i == streams.size() - 1 ? ctrl : ctrl->dup(), outGateBaseId + i);
    }
}

/*
 * Send data from the generator to the quic layer
 */
void QuicTrafficgen::sendData(TrafficgenData* gmsg)
{
    EV_INFO << "sendData - stream " << gmsg->getId() << " - size " << gmsg->getByteLength() << " byte" << endl;

    Packet *packet = new Packet("ApplicationData");
    auto applicationData = makeShared<ByteCountChunk>(B(gmsg->getByteLength()));

    // set streamId for applicationData
    auto& tags = packet->getTags();
    tags.addTagIfAbsent<QuicStreamReq>()->setStreamID(getStreamId(gmsg->getId()));

    packet->insertAtBack(applicationData);
    //emit(packetSentSignal, packet);
    socket.send(packet);
}

void QuicTrafficgen::close() {
    EV_INFO << "sendShutdown" << endl;
    socket.close();
    //socket.abort();
}

void QuicTrafficgen::setStatusString(const char *s) {
    if (hasGUI()) {
        //getDisplayString().setTagArg("t", 0, s);
    }
}

void QuicTrafficgen::socketSendQueueFull(QuicSocket *socket)
{
    sendGeneratorControl(TRAFFICGEN_STOP_SENDING);
    sendingAllowed = false;
}

void QuicTrafficgen::socketSendQueueDrain(QuicSocket *socket)
{
    sendGeneratorControl(TRAFFICGEN_START_SENDING);
    sendingAllowed = true;
}

uint32_t QuicTrafficgen::getStreamId(int generatorId){
    if(generatorId < 0) throw cRuntimeError("Invalid generatorId: %d, %s", (int) generatorId, "correct ini file!");
    return generatorId;
}
} //namespace
