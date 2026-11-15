// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/applications/mqtt/MqttBroker.h"

#include "inet/applications/mqtt/MqttProtocol.h"
#include "inet/applications/mqtt/MqttTopicFilter.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet::mqtt {
Define_Module(MqttBroker);

MqttBroker::~MqttBroker()
{
    cancelAndDelete(timeoutTimer);
    socketMap.deleteSockets();
}
void MqttBroker::initialize(int stage)
{
    ApplicationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        timeoutTimer = new cMessage("connectionDeadlines");
        listener.setCallback(this); listener.setOutputGate(gate("socketOut"));
        if (par("localPort").intValue() < 1 || par("localPort").intValue() > 65535 || par("connectTimeout").doubleValue() <= 0 || par("stopOperationTimeout").doubleValue() <= 0)
            throw cRuntimeError("Invalid MQTT broker port or timeout");
    }
}
void MqttBroker::handleStartOperation(LifecycleOperation *)
{
    listener.renewSocket();
    const char *address = par("localAddress");
    listener.bind(*address ? L3AddressResolver().resolve(address) : L3Address(), par("localPort"));
    listener.listen();
}
void MqttBroker::socketAvailable(TcpSocket *socket, TcpAvailableInfo *info)
{
    auto accepted = new TcpSocket(info);
    accepted->setOutputGate(gate("socketOut")); accepted->setCallback(this);
    socketMap.addSocket(accepted);
    auto& connection = connections[accepted->getSocketId()];
    connection.deadline = simTime() + par("connectTimeout");
    socket->accept(info->getNewSocketId());
    scheduleTimeout();
}
void MqttBroker::sendControl(TcpSocket *socket, const Ptr<MqttPacket>& packet)
{
    MqttPacketSerializer::setLength(packet);
    auto message = new Packet("mqtt", packet);
    message->addTag<PacketProtocolTag>()->setProtocol(&mqttProtocol);
    emit(registerSignal("packetSentToLower"), message);
    socket->send(message);
}
void MqttBroker::receiveControl(TcpSocket *socket, const Ptr<MqttPacket>& packet)
{
    auto& connection = connections.at(socket->getSocketId());
    if (connection.phase == MqttConnection::WAITING) {
        if (packet->getType() != CONNECT || std::string(packet->getProtocolName()) != "MQTT")
            throw cRuntimeError("Expected MQTT CONNECT");
        int refusal = packet->getProtocolLevel() != 4 ? 1 : 0;
        std::string id = packet->getClientIdentifier();
        if (!refusal && (id.empty() || id.size() > 23 || id.find_first_not_of("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos)) refusal = 2;
        std::string password;
        for (size_t i = 0; i < packet->getPasswordArraySize(); i++) password.push_back(packet->getPassword(i));
        if (!refusal && *par("userName").stringValue() &&
                (!packet->getUserNamePresent() || !packet->getPasswordPresent() ||
                std::string(packet->getUserName()) != par("userName").stdstringValue() || password != par("password").stdstringValue())) refusal = 4;
        auto response = makeShared<MqttPacket>(); response->setType(CONNACK); response->setReturnCode(refusal);
        if (refusal) { sendControl(socket, response); closeConnection(socket); return; }
        if (packet->getKeepAlive() || packet->getWill())
            throw cRuntimeError("Requested MQTT behavior is not supported in this implementation stage");
        // Section 3.1.4: disconnect an old connection before activating its replacement.
        for (auto& pair : connections) {
            if (pair.first != socket->getSocketId() && pair.second.phase == MqttConnection::ACTIVE && pair.second.clientIdentifier == id)
                closeConnection(check_and_cast<TcpSocket *>(socketMap.getSocketById(pair.first)));
        }
        bool present = sessions.find(id) != sessions.end() && !packet->getCleanSession();
        if (!present) sessions[id] = MqttSession();
        response->setSessionPresent(present);
        connection.clientIdentifier = id; connection.cleanSession = packet->getCleanSession();
        connection.phase = MqttConnection::ACTIVE;
        sendControl(socket, response);
        for (auto& outgoing : sessions.at(id).retry()) sendControl(socket, outgoing);
        for (auto& outgoing : sessions.at(id).flush()) sendControl(socket, outgoing);
        emit(registerSignal("connectionChanged"), 1L);
        scheduleTimeout(); return;
    }
    auto& session = sessions.at(connection.clientIdentifier);
    if (packet->getType() == SUBSCRIBE || packet->getType() == UNSUBSCRIBE) {
        auto response = makeShared<MqttPacket>();
        response->setType(packet->getType() == SUBSCRIBE ? SUBACK : UNSUBACK);
        response->setPacketIdentifier(packet->getPacketIdentifier());
        if (packet->getType() == SUBSCRIBE) response->setResultsArraySize(packet->getFiltersArraySize());
        for (size_t i = 0; i < packet->getFiltersArraySize(); i++) {
            std::string filter = packet->getFilters(i);
            if (!MqttTopicFilter::isValidFilter(filter)) throw cRuntimeError("Invalid MQTT filter");
            if (packet->getType() == SUBSCRIBE) {
                int granted = std::min(packet->getResults(i), 1);
                session.subscriptions[filter] = granted; response->setResults(i, granted);
            }
            else session.subscriptions.erase(filter);
        }
        sendControl(socket, response);
    }
    else if (packet->getType() == PUBLISH && packet->getQos() <= 1 && !packet->getRetain()) {
        if (!MqttTopicFilter::isValidTopic(packet->getTopic())) throw cRuntimeError("Invalid MQTT topic");
        route(packet);
        if (packet->getQos()) {
            auto ack = makeShared<MqttPacket>(); ack->setType(PUBACK); ack->setPacketIdentifier(packet->getPacketIdentifier()); sendControl(socket, ack);
        }
    }
    else if (packet->getType() == PUBACK) {
        session.acknowledge(packet);
        for (auto& outgoing : session.flush()) sendControl(socket, outgoing);
    }
    else if (packet->getType() == DISCONNECT) closeConnection(socket);
    else throw cRuntimeError("Unexpected MQTT packet in active broker connection");
}
void MqttBroker::route(const Ptr<const MqttPacket>& packet)
{
    for (auto& pair : sessions) {
        auto& session = pair.second;
        int granted = -1;
        for (auto& subscription : session.subscriptions)
            if (MqttTopicFilter::matches(subscription.first, packet->getTopic())) granted = std::max(granted, subscription.second);
        TcpSocket *recipient = nullptr;
        for (auto& connection : connections)
            if (connection.second.phase == MqttConnection::ACTIVE && connection.second.clientIdentifier == pair.first)
                recipient = check_and_cast<TcpSocket *>(socketMap.getSocketById(connection.first));
        int qos = std::min(granted, packet->getQos());
        if (granted >= 0 && (recipient || qos > 0)) {
            auto copy = staticPtrCast<MqttPacket>(packet->dupShared());
            copy->setRetain(false); copy->setDuplicate(false); copy->setQos(qos); copy->setPacketIdentifier(0);
            session.enqueue(copy);
            if (recipient) for (auto& outgoing : session.flush()) sendControl(recipient, outgoing);
        }
    }
}
void MqttBroker::closeConnection(TcpSocket *socket)
{
    auto found = connections.find(socket->getSocketId());
    if (found == connections.end() || found->second.phase == MqttConnection::CLOSING) return;
    auto& connection = found->second;
    connection.phase = MqttConnection::CLOSING;
    if (connection.cleanSession) sessions.erase(connection.clientIdentifier);
    socket->getReadBuffer()->clear();
    if (socket->getState() == TcpSocket::CONNECTED || socket->getState() == TcpSocket::PEER_CLOSED) socket->close();
    else if (socket->isOpen()) socket->abort();
    emit(registerSignal("connectionChanged"), 0L);
    scheduleTimeout();
}
void MqttBroker::socketDataArrived(TcpSocket *socket, Packet *message, bool)
{
    auto& connection = connections.at(socket->getSocketId());
    if (connection.phase == MqttConnection::CLOSING) { delete message; return; }
    socket->getReadBuffer()->push(message->peekData()); delete message;
    try {
        while (auto packet = MqttConnection::extract(*socket->getReadBuffer())) {
            Packet observation("mqttReceived", packet); emit(registerSignal("packetReceivedFromLower"), &observation);
            receiveControl(socket, packet);
            if (connection.phase == MqttConnection::CLOSING) break;
        }
    }
    catch (cRuntimeError& error) { EV_WARN << error.what() << endl; closeConnection(socket); }
}
void MqttBroker::scheduleTimeout()
{
    cancelEvent(timeoutTimer);
    simtime_t next = SimTime::getMaxTime();
    for (auto& pair : connections)
        if (pair.second.phase == MqttConnection::WAITING) next = std::min(next, pair.second.deadline);
    if (next != SimTime::getMaxTime()) scheduleAt(std::max(simTime(), next), timeoutTimer);
}
void MqttBroker::handleMessageWhenUp(cMessage *message)
{
    if (message == timeoutTimer) {
        for (auto& pair : connections)
            if (pair.second.phase == MqttConnection::WAITING && pair.second.deadline <= simTime())
                closeConnection(check_and_cast<TcpSocket *>(socketMap.getSocketById(pair.first)));
        scheduleTimeout();
    }
    else if (auto socket = check_and_cast_nullable<TcpSocket *>(socketMap.findSocketFor(message))) socket->processMessage(message);
    else if (listener.belongsToSocket(message)) listener.processMessage(message);
    else delete message;
}
void MqttBroker::handleStopOperation(LifecycleOperation *)
{
    for (auto& pair : connections) closeConnection(check_and_cast<TcpSocket *>(socketMap.getSocketById(pair.first)));
    listener.close(); cancelEvent(timeoutTimer);
}
void MqttBroker::handleCrashOperation(LifecycleOperation *)
{
    cancelEvent(timeoutTimer);
    for (auto& pair : socketMap.getMap()) pair.second->destroy();
    listener.destroy();
    socketMap.deleteSockets(); connections.clear(); sessions.clear();
}
void MqttBroker::socketPeerClosed(TcpSocket *socket) { closeConnection(socket); }
void MqttBroker::socketClosed(TcpSocket *socket) { closeConnection(socket); }
void MqttBroker::socketFailure(TcpSocket *socket, int) { closeConnection(socket); }
} // namespace inet::mqtt
