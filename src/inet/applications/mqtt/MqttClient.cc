// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/applications/mqtt/MqttClient.h"

#include "inet/applications/mqtt/MqttProtocol.h"
#include "inet/applications/mqtt/MqttTopicFilter.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet::mqtt {
Define_Module(MqttClient);

namespace {
std::vector<uint8_t> mqttHexPayload(const std::string& text)
{
    if (text.size() % 2) throw cRuntimeError("MQTT hexadecimal payload has an odd number of digits");
    std::vector<uint8_t> bytes;
    auto digit = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        throw cRuntimeError("Invalid MQTT hexadecimal payload");
    };
    for (size_t i = 0; i < text.size(); i += 2) bytes.push_back((digit(text[i]) << 4) | digit(text[i + 1]));
    return bytes;
}
}

MqttClient::~MqttClient()
{
    cancelAndDelete(connectTimer);
    cancelAndDelete(timeoutTimer);
    cancelAndDelete(workTimer);
}

void MqttClient::initialize(int stage)
{
    ApplicationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        connectTimer = new cMessage("connect"); timeoutTimer = new cMessage("connectionTimeout"); workTimer = new cMessage("work");
        socket.setCallback(this); socket.setOutputGate(gate("socketOut"));
        if (par("connectPort").intValue() < 1 || par("connectPort").intValue() > 65535 ||
                par("startTime").doubleValue() < 0 || par("reconnectInterval").doubleValue() <= 0 ||
                par("connectTimeout").doubleValue() <= 0 || par("pingResponseTimeout").doubleValue() <= 0 || par("stopOperationTimeout").doubleValue() <= 0)
            throw cRuntimeError("Invalid MQTT port or timing parameter");
        if (par("keepAlive").intValue() != 0 || *par("willTopic").stringValue())
            throw cRuntimeError("This MQTT stage supports keepalive zero and no Will");
        readWorkload();
    }
}

void MqttClient::readWorkload()
{
    actions.clear();
    for (auto element : par("workload").xmlValue()->getChildren()) {
        auto attribute = [&](const char *name, const char *fallback = nullptr) -> const char * {
            const char *value = element->getAttribute(name);
            if (!value) value = fallback;
            if (!value) throw cRuntimeError("MQTT workload action lacks '%s'", name);
            return value;
        };
        auto packet = makeShared<MqttPacket>();
        std::string name = element->getTagName();
        int qos = std::stoi(attribute("qos", "0"));
        if (qos < 0 || qos > 1) throw cRuntimeError("This MQTT stage supports QoS 0 and 1");
        if (name == "publish") {
            packet->setType(PUBLISH); packet->setQos(qos); packet->setTopic(attribute("topic"));
            packet->setRetain(std::string(attribute("retain", "false")) == "true");
            if (packet->getRetain()) throw cRuntimeError("Retained messages are not implemented in this stage");
            if (!MqttTopicFilter::isValidTopic(packet->getTopic())) throw cRuntimeError("Invalid MQTT workload topic");
            auto bytes = mqttHexPayload(attribute("payload", "")); packet->setPayloadArraySize(bytes.size());
            for (size_t i = 0; i < bytes.size(); i++) packet->setPayload(i, bytes[i]);
        }
        else if (name == "subscribe" || name == "unsubscribe") {
            packet->setType(name == "subscribe" ? SUBSCRIBE : UNSUBSCRIBE);
            packet->setFiltersArraySize(1); packet->setFilters(0, attribute("filter"));
            if (!MqttTopicFilter::isValidFilter(packet->getFilters(0))) throw cRuntimeError("Invalid MQTT workload filter");
            if (packet->getType() == SUBSCRIBE) { packet->setResultsArraySize(1); packet->setResults(0, qos); }
        }
        else throw cRuntimeError("Unknown MQTT workload action '%s'", name.c_str());
        simtime_t at = SimTime::parse(attribute("at"));
        if (at < SIMTIME_ZERO || (!actions.empty() && at < actions.back().offset))
            throw cRuntimeError("MQTT workload times must be nonnegative and ordered");
        actions.push_back({at, packet});
    }
}

void MqttClient::handleStartOperation(LifecycleOperation *)
{
    if (*par("connectAddress").stringValue())
        scheduleAt(std::max(simTime(), simtime_t(par("startTime"))), connectTimer);
}

void MqttClient::connect()
{
    socket.getReadBuffer()->clear();
    socket.renewSocket();
    const char *local = par("localAddress");
    socket.bind(*local ? L3AddressResolver().resolve(local) : L3Address(), par("localPort"));
    connection = MqttConnection();
    connection.cleanSession = par("cleanSession");
    socket.connect(L3AddressResolver().resolve(par("connectAddress")), par("connectPort"));
    scheduleAt(simTime() + par("connectTimeout"), timeoutTimer);
}

void MqttClient::sendControl(const Ptr<MqttPacket>& packet)
{
    MqttPacketSerializer::setLength(packet);
    auto message = new Packet("mqtt", packet);
    message->addTag<PacketProtocolTag>()->setProtocol(&mqttProtocol);
    emit(registerSignal("packetSentToLower"), message);
    socket.send(message);
}

void MqttClient::socketEstablished(TcpSocket *)
{
    auto packet = makeShared<MqttPacket>(); packet->setType(CONNECT);
    packet->setClientIdentifier(par("clientIdentifier")); packet->setCleanSession(connection.cleanSession);
    std::string user = par("userName"), password = par("password");
    packet->setUserNamePresent(!user.empty() || !password.empty()); packet->setUserName(user.c_str());
    packet->setPasswordPresent(packet->getUserNamePresent()); packet->setPasswordArraySize(password.size());
    for (size_t i = 0; i < password.size(); i++) packet->setPassword(i, password[i]);
    sendControl(packet);
}

void MqttClient::scheduleWork()
{
    if (connection.phase != MqttConnection::ACTIVE || pendingSubscription || actionIndex == actions.size() || workTimer->isScheduled()) return;
    scheduleAt(std::max(simTime(), workloadOrigin + actions[actionIndex].offset), workTimer);
}

void MqttClient::runWork()
{
    auto packet = staticPtrCast<MqttPacket>(actions[actionIndex].packet->dupShared());
    if (packet->getType() == SUBSCRIBE || packet->getType() == UNSUBSCRIBE) {
        int identifier = session.allocateIdentifier();
        if (!identifier) return;
        packet->setPacketIdentifier(identifier); pendingSubscription = packet;
        if (packet->getType() == SUBSCRIBE) session.subscriptions[packet->getFilters(0)] = packet->getResults(0);
        else session.subscriptions.erase(packet->getFilters(0));
    }
    actionIndex++;
    if (packet->getType() == PUBLISH) {
        session.enqueue(packet);
        for (auto& outgoing : session.flush()) sendControl(outgoing);
    }
    else sendControl(packet);
    scheduleWork();
}

void MqttClient::sendPendingSubscription()
{
    if (!pendingSubscription || pendingSubscription->getPacketIdentifier()) return;
    int identifier = session.allocateIdentifier();
    if (!identifier) return;
    auto packet = staticPtrCast<MqttPacket>(pendingSubscription->dupShared());
    packet->setPacketIdentifier(identifier); pendingSubscription = packet;
    sendControl(packet);
}

void MqttClient::receiveControl(const Ptr<MqttPacket>& packet)
{
    if (connection.phase == MqttConnection::WAITING) {
        if (packet->getType() != CONNACK || (connection.cleanSession && packet->getSessionPresent()))
            throw cRuntimeError("Expected MQTT CONNACK with consistent Session Present");
        if (packet->getReturnCode() != 0) { failConnection(); return; }
        cancelEvent(timeoutTimer); connection.phase = MqttConnection::ACTIVE;
        if (workloadOrigin < SIMTIME_ZERO) workloadOrigin = simTime();
        if (!packet->getSessionPresent()) {
            pendingSubscription = nullptr;
            if (!session.subscriptions.empty()) {
                auto restore = makeShared<MqttPacket>(); restore->setType(SUBSCRIBE);
                restore->setFiltersArraySize(session.subscriptions.size()); restore->setResultsArraySize(session.subscriptions.size());
                size_t i = 0;
                for (auto& subscription : session.subscriptions) {
                    restore->setFilters(i, subscription.first.c_str()); restore->setResults(i++, subscription.second);
                }
                pendingSubscription = restore;
            }
        }
        // Recovery uses existing identifiers and must not wait for a new SUBSCRIBE identifier.
        for (auto& outgoing : session.retry()) sendControl(outgoing);
        sendPendingSubscription();
        for (auto& outgoing : session.flush()) sendControl(outgoing);
        emit(registerSignal("connectionChanged"), 1L);
        scheduleWork(); return;
    }
    if (packet->getType() == SUBACK || packet->getType() == UNSUBACK) {
        if (!pendingSubscription || packet->getPacketIdentifier() != pendingSubscription->getPacketIdentifier() ||
                packet->getType() != (pendingSubscription->getType() == SUBSCRIBE ? SUBACK : UNSUBACK) ||
                (packet->getType() == SUBACK && packet->getResultsArraySize() != pendingSubscription->getFiltersArraySize()))
            throw cRuntimeError("Unexpected MQTT subscription acknowledgement");
        if (packet->getType() == SUBACK && packet->getResults(0) == 128) EV_WARN << "MQTT subscription refused\n";
        if (packet->getType() == SUBACK)
            for (size_t i = 0; i < packet->getResultsArraySize(); i++)
                if (packet->getResults(i) != 128 && packet->getResults(i) > pendingSubscription->getResults(i))
                    throw cRuntimeError("MQTT broker granted a higher QoS than requested");
        session.releaseIdentifier(packet->getPacketIdentifier()); pendingSubscription = nullptr;
        for (auto& outgoing : session.flush()) sendControl(outgoing);
        scheduleWork();
    }
    else if (packet->getType() == PUBACK) {
        session.acknowledge(packet);
        sendPendingSubscription();
        for (auto& outgoing : session.flush()) sendControl(outgoing);
        scheduleWork();
    }
    else if (packet->getType() == PUBLISH && packet->getQos() <= 1 && MqttTopicFilter::isValidTopic(packet->getTopic())) {
        deliverPublication(packet);
        if (packet->getQos()) {
            auto ack = makeShared<MqttPacket>(); ack->setType(PUBACK); ack->setPacketIdentifier(packet->getPacketIdentifier()); sendControl(ack);
        }
    }
    else throw cRuntimeError("Unexpected MQTT packet in active client");
}

void MqttClient::deliverPublication(const Ptr<const MqttPacket>& packet)
{
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < packet->getPayloadArraySize(); i++) bytes.push_back(packet->getPayload(i));
    Packet delivered("publication", makeShared<BytesChunk>(bytes));
    emit(registerSignal("packetSentToUpper"), &delivered);
}

void MqttClient::socketDataArrived(TcpSocket *, Packet *message, bool)
{
    socket.getReadBuffer()->push(message->peekData()); delete message;
    if (connection.phase == MqttConnection::CLOSING) { socket.getReadBuffer()->clear(); return; }
    try {
        while (auto packet = MqttConnection::extract(*socket.getReadBuffer())) {
            Packet observation("mqttReceived", packet);
            emit(registerSignal("packetReceivedFromLower"), &observation);
            receiveControl(packet);
            if (connection.phase == MqttConnection::CLOSING) break;
        }
    }
    catch (cRuntimeError& error) { EV_WARN << error.what() << endl; failConnection(); }
}

void MqttClient::failConnection()
{
    connection.phase = MqttConnection::CLOSING;
    cancelEvent(timeoutTimer); cancelEvent(workTimer);
    socket.getReadBuffer()->clear();
    if (socket.isOpen()) socket.abort();
    if (pendingSubscription) {
        session.releaseIdentifier(pendingSubscription->getPacketIdentifier());
        pendingSubscription = staticPtrCast<MqttPacket>(pendingSubscription->dupShared()); pendingSubscription->setPacketIdentifier(0);
    }
    if (connection.cleanSession) session.clearExchanges();
    emit(registerSignal("connectionChanged"), 0L);
    if (operationalState == State::OPERATING && !connectTimer->isScheduled()) scheduleAt(simTime() + par("reconnectInterval"), connectTimer);
    else if (operationalState == State::STOPPING_OPERATION) finishActiveOperation();
}

void MqttClient::disconnect()
{
    cancelEvent(connectTimer); cancelEvent(timeoutTimer); cancelEvent(workTimer);
    if (connection.phase == MqttConnection::ACTIVE) {
        auto packet = makeShared<MqttPacket>(); packet->setType(DISCONNECT); sendControl(packet);
    }
    connection.phase = MqttConnection::CLOSING;
    if (socket.getState() == TcpSocket::CONNECTED || socket.getState() == TcpSocket::PEER_CLOSED) socket.close();
    else if (socket.isOpen()) socket.abort();
}

void MqttClient::handleStopOperation(LifecycleOperation *)
{
    delayActiveOperationFinish(par("stopOperationTimeout"));
    disconnect();
    if (!socket.isOpen()) finishActiveOperation();
}
void MqttClient::handleCrashOperation(LifecycleOperation *)
{
    cancelEvent(connectTimer); cancelEvent(timeoutTimer); cancelEvent(workTimer);
    socket.destroy(); socket.getReadBuffer()->clear(); connection.phase = MqttConnection::CLOSING;
}
void MqttClient::handleMessageWhenUp(cMessage *message)
{
    if (message == connectTimer) connect();
    else if (message == timeoutTimer) failConnection();
    else if (message == workTimer) runWork();
    else if (socket.belongsToSocket(message)) socket.processMessage(message);
    else delete message;
}
void MqttClient::socketAvailable(TcpSocket *, TcpAvailableInfo *) { throw cRuntimeError("MQTT client cannot accept connections"); }
void MqttClient::socketPeerClosed(TcpSocket *) { failConnection(); }
void MqttClient::socketClosed(TcpSocket *) { failConnection(); }
void MqttClient::socketFailure(TcpSocket *, int) { failConnection(); }
} // namespace inet::mqtt
