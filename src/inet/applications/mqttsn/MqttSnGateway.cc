// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/applications/mqttsn/MqttSnGateway.h"
#include "inet/applications/mqttsn/MqttSnPacketSerializer.h"
#include "inet/applications/mqtt/MqttProtocol.h"
#include "inet/applications/mqtt/MqttTopicFilter.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

namespace inet::mqttsn {
Define_Module(MqttSnGateway);

MqttSnGateway::~MqttSnGateway() { cancelAndDelete(timer); brokerSockets.deleteSockets(); }

void MqttSnGateway::initialize(int stage)
{
    ApplicationBase::initialize(stage);
    if (stage != INITSTAGE_LOCAL) return;
    timer = new cMessage("mqttSnGatewayDeadlines");
    if (!*par("brokerAddress").stringValue() || par("gatewayId").intValue() < 0 || par("gatewayId").intValue() > 255 ||
            par("advertisementInterval").intValue() < 1 || par("advertisementInterval").intValue() > 65535 ||
            par("retryInterval").doubleValue() <= 0 || par("retryCount").intValue() < 0 || par("congestionWait").doubleValue() <= 0 ||
            par("connectTimeout").doubleValue() <= 0 || par("reconnectInterval").doubleValue() <= 0 ||
            par("brokerKeepAlive").intValue() < 0 || par("brokerKeepAlive").intValue() > 65535 || par("brokerPingTimeout").doubleValue() <= 0)
        throw cRuntimeError("Invalid MQTT-SN gateway configuration");
    shared.clientIdentifier = par("gatewayClientIdentifier").stdstringValue(); shared.cleanSession = false;
    MqttSnSession::readPredefined(par("predefinedTopics").xmlValue(), shared.predefinedTopics);
}

void MqttSnGateway::handleStartOperation(LifecycleOperation *)
{
    socket.setOutputGate(gate("socketOut")); socket.setCallback(this);
    const char *local = par("localAddress"); socket.bind(*local ? L3AddressResolver().resolve(local) : L3Address(), par("localPort"));
    socket.setBroadcast(true); bound = true;
    sharedReconnectAt = advertiseAt = simTime(); openBroker(shared); scheduleTimer();
}

void MqttSnGateway::sendTo(const Ptr<MqttSnPacket>& packet, const L3Address& address, int port)
{
    MqttSnPacketSerializer::setLength(packet);
    if (packet->getChunkLength() > B(65507)) { EV_WARN << "MQTT-SN publication exceeds UDP profile limit\n"; return; }
    auto message = new Packet("mqttsn", packet); message->addTag<PacketProtocolTag>()->setProtocol(&mqttSnProtocol);
    if (packet->getType() == ADVERTISE || packet->getType() == GWINFO) {
        auto table = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        auto interface = table->findInterfaceByName(par("discoveryInterface"));
        if (!interface) { delete message; throw cRuntimeError("Unknown MQTT-SN discovery interface"); }
        message->addTag<InterfaceReq>()->setInterfaceId(interface->getInterfaceId());
        message->addTag<HopLimitReq>()->setHopLimit(1);
    }
    emit(registerSignal("packetSentToLower"), message); socket.sendTo(message, address, port);
}

void MqttSnGateway::transmit(Peer& peer, const Ptr<MqttSnPacket>& packet, bool track)
{
    if (packet->getType() == PUBLISH && qos(*packet) == 0) peer.qosZeroTopics[packet->getTopicId()] = topicKind(*packet);
    if (track) {
        int key = packet->getMessageId() ? packet->getMessageId() : -packet->getType();
        peer.transactions[key] = {packet, simTime() + par("retryInterval"), 0};
    }
    sendTo(packet, peer.address, peer.port);
}

MqttSnGateway::Peer *MqttSnGateway::findPeer(TcpSocket *socket)
{
    if (shared.socketId == socket->getSocketId()) return &shared;
    for (auto& entry : peers) if (entry.second.socketId == socket->getSocketId()) return &entry.second;
    return nullptr;
}

void MqttSnGateway::openBroker(Peer& peer)
{
    if (peer.socketId >= 0) return;
    auto connection = new TcpSocket(); connection->setOutputGate(gate("socketOut")); connection->setCallback(this);
    brokerSockets.addSocket(connection); peer.socketId = connection->getSocketId(); peer.brokerActive = false;
    peer.brokerDeadline = simTime() + par("connectTimeout");
    connection->connect(L3AddressResolver().resolve(par("brokerAddress")), par("brokerPort"));
}

void MqttSnGateway::closeBroker(Peer& peer)
{
    auto found = brokerSockets.getMap().find(peer.socketId);
    auto connection = found == brokerSockets.getMap().end() ? nullptr : check_and_cast<TcpSocket *>(found->second);
    peer.socketId = -1; peer.brokerActive = false; peer.brokerDeadline = peer.brokerPingDeadline = -1;
    if (!connection) return;
    connection->getReadBuffer()->clear();
    if (connection->getState() == TcpSocket::CONNECTED || connection->getState() == TcpSocket::PEER_CLOSED) connection->close();
    else if (connection->isOpen()) connection->abort();
}

void MqttSnGateway::failBroker(Peer& peer, int refusal)
{
    closeBroker(peer);
    if (&peer == &shared) sharedReconnectAt = simTime() + par("reconnectInterval");
    else {
        if (bound) {
            bool setup = peer.phase == CONNECTING || peer.phase == WILL_TOPIC || peer.phase == WILL_MESSAGE;
            auto reply = MqttSnSession::makePacket(setup ? CONNACK : DISCONNECT);
            if (setup) reply->setReturnCode(refusal);
            transmit(peer, reply);
        }
        disconnectPeer(peer, false);
    }
}

void MqttSnGateway::brokerSend(Peer& peer, const Ptr<mqtt::MqttPacket>& packet)
{
    auto found = brokerSockets.getMap().find(peer.socketId);
    auto connection = found == brokerSockets.getMap().end() ? nullptr : check_and_cast<TcpSocket *>(found->second);
    if (!connection) return;
    mqtt::MqttPacketSerializer::setLength(packet);
    auto message = new Packet("mqtt", packet); message->addTag<PacketProtocolTag>()->setProtocol(&mqtt::mqttProtocol);
    peer.brokerLastSent = simTime(); emit(registerSignal("packetSentToLower"), message); connection->send(message);
}

void MqttSnGateway::socketEstablished(TcpSocket *socket)
{
    auto peer = findPeer(socket); if (!peer) return;
    auto packet = makeShared<mqtt::MqttPacket>(); packet->setType(mqtt::CONNECT);
    packet->setClientIdentifier(peer->clientIdentifier.c_str()); packet->setCleanSession(peer->cleanSession);
    packet->setKeepAlive(par("brokerKeepAlive"));
    packet->setUserNamePresent(*par("userName").stringValue()); packet->setUserName(par("userName"));
    packet->setPasswordPresent(*par("password").stringValue());
    std::string password = par("password").stdstringValue(); packet->setPasswordArraySize(password.size());
    for (size_t i = 0; i < password.size(); i++) packet->setPassword(i, password[i]);
    brokerSend(*peer, packet);
}

void MqttSnGateway::finishSetup(Peer& peer)
{
    peer.phase = CONNECTING; peer.transactions.erase(-WILLTOPICREQ); peer.transactions.erase(-WILLMSGREQ); openBroker(peer);
}

void MqttSnGateway::acceptConnect(const Ptr<MqttSnPacket>& packet, const L3Address& source, int port)
{
    if (packet->getProtocolId() != 1 || std::string(packet->getClientIdentifier()) == shared.clientIdentifier) {
        auto refusal = MqttSnSession::makePacket(CONNACK); refusal->setReturnCode(NOT_SUPPORTED); sendTo(refusal, source, port); return;
    }
    auto& peer = peers[packet->getClientIdentifier()];
    if (peer.connectRequest && peer.address == source && peer.port == port && MqttSnSession::sameRequest(*peer.connectRequest, *packet) &&
            peer.phase != DISCONNECTED && peer.phase != LOST && peer.phase != ASLEEP && peer.phase != AWAKE) {
        if (peer.phase == ACTIVE) transmit(peer, MqttSnSession::makePacket(CONNACK));
        else if (peer.phase == WILL_TOPIC || peer.phase == WILL_MESSAGE)
            transmit(peer, MqttSnSession::makePacket(peer.phase == WILL_TOPIC ? WILLTOPICREQ : WILLMSGREQ));
        return;
    }
    closeBroker(peer);
    for (const auto& entry : peer.pendingSubscriptions) peer.mqttSession.releaseIdentifier(entry.first);
    for (const auto& entry : peer.restoring) peer.mqttSession.releaseIdentifier(entry.first);
    peer.pendingSubscriptions.clear(); peer.restoring.clear(); peer.restoreQueue.clear();
    bool clean = packet->getFlags() & CLEAN_SESSION;
    if (clean) { peer = Peer(); peer.predefinedTopics = shared.predefinedTopics; }
    peer.predefinedTopics = shared.predefinedTopics;
    peer.address = source; peer.port = port; peer.clientIdentifier = packet->getClientIdentifier();
    peer.cleanSession = clean; peer.keepAlive = packet->getDuration(); peer.connectRequest = packet;
    peer.deadline = -1;
    for (auto& entry : peer.transactions) entry.second.deadline = SimTime::getMaxTime();
    if (packet->getFlags() & WILL) {
        peer.phase = WILL_TOPIC; transmit(peer, MqttSnSession::makePacket(WILLTOPICREQ), true);
    }
    else finishSetup(peer);
}

void MqttSnGateway::rememberReply(Peer& peer, const Ptr<MqttSnPacket>& request, const Ptr<MqttSnPacket>& reply)
{
    peer.replies[request->getType()] = {request, reply}; transmit(peer, reply);
}

bool MqttSnGateway::replayReply(Peer& peer, const Ptr<MqttSnPacket>& request)
{
    if (request->getType() == SUBSCRIBE && !(request->getFlags() & DUPLICATE)) return false;
    auto found = peer.replies.find(request->getType());
    if (found != peer.replies.end() && MqttSnSession::sameRequest(*request, *found->second.first)) {
        transmit(peer, staticPtrCast<MqttSnPacket>(found->second.second->dupShared())); return true;
    }
    return false;
}

void MqttSnGateway::enqueueBroker(Peer& peer, const Ptr<MqttSnPacket>& packet, const std::string& topic)
{
    auto publication = makeShared<mqtt::MqttPacket>(); publication->setType(mqtt::PUBLISH); publication->setTopic(topic.c_str());
    publication->setQos(std::max(0, qos(*packet))); publication->setRetain(packet->getFlags() & RETAIN);
    publication->setPayloadArraySize(packet->getPayloadArraySize());
    for (size_t i = 0; i < packet->getPayloadArraySize(); i++) publication->setPayload(i, packet->getPayload(i));
    peer.mqttSession.enqueue(publication);
    if (peer.brokerActive) restoreSubscriptions(peer);
}

void MqttSnGateway::disconnectPeer(Peer& peer, bool lost)
{
    if (lost && peer.phase != LOST && peer.phase != DISCONNECTED && peer.will)
        enqueueBroker(shared, peer.will, peer.will->getTopic());
    peer.phase = lost ? LOST : DISCONNECTED; peer.deadline = -1;
    if (peer.brokerActive) { auto bye = makeShared<mqtt::MqttPacket>(); bye->setType(mqtt::DISCONNECT); brokerSend(peer, bye); }
    closeBroker(peer);
    for (const auto& entry : peer.pendingSubscriptions) peer.mqttSession.releaseIdentifier(entry.first);
    for (const auto& entry : peer.restoring) peer.mqttSession.releaseIdentifier(entry.first);
    peer.pendingSubscriptions.clear(); peer.restoring.clear(); peer.restoreQueue.clear();
    if (peer.cleanSession) {
        peer.clearExchanges(); peer.topics.clear(); peer.subscriptions.clear(); peer.excludedTopics.clear(); peer.will = nullptr; peer.mqttSession.clearExchanges();
    }
    else for (auto& entry : peer.transactions) entry.second.deadline = SimTime::getMaxTime();
    emit(registerSignal("sessionStateChanged"), long(peer.phase));
}

void MqttSnGateway::receiveControlPacket(Peer& peer, const Ptr<MqttSnPacket>& packet)
{
    int type = packet->getType(), id = packet->getMessageId();
    if (type == WILLTOPIC && peer.phase == WILL_TOPIC) {
        if (!packet->getWillTopicPresent()) { peer.transactions.erase(-WILLTOPICREQ); peer.will = nullptr; finishSetup(peer); return; }
        if (!mqtt::MqttTopicFilter::isValidTopic(packet->getTopic()) || qos(*packet) < 0) return;
        peer.transactions.erase(-WILLTOPICREQ);
        peer.will = MqttSnSession::makePacket(PUBLISH); peer.will->setTopic(packet->getTopic());
        peer.will->setFlags(packet->getFlags() & (0x60 | RETAIN)); peer.phase = WILL_MESSAGE;
        transmit(peer, MqttSnSession::makePacket(WILLMSGREQ), true); return;
    }
    if (type == WILLMSG && peer.phase == WILL_MESSAGE) {
        MqttSnSession::setPayload(peer.will, MqttSnSession::payload(*packet)); peer.transactions.erase(-WILLMSGREQ); finishSetup(peer); return;
    }
    if (type == WILLTOPIC && peer.phase == WILL_MESSAGE) { transmit(peer, MqttSnSession::makePacket(WILLMSGREQ)); return; }
    if ((type == WILLTOPIC || type == WILLMSG) && (peer.phase == CONNECTING || peer.phase == ACTIVE)) return;
    if (type == DISCONNECT) {
        if (packet->getDurationPresent() && (peer.phase == ACTIVE || peer.phase == ASLEEP || peer.phase == AWAKE)) {
            peer.phase = ASLEEP; peer.sleepDuration = packet->getDuration();
            peer.deadline = simTime() + SimTime(peer.sleepDuration * (peer.sleepDuration <= 60 ? 1.5 : 1.1));
            for (auto& entry : peer.transactions) entry.second.deadline = SimTime::getMaxTime();
        }
        else disconnectPeer(peer, false);
        transmit(peer, MqttSnSession::makePacket(DISCONNECT)); return;
    }
    if (peer.phase != ACTIVE && peer.phase != ASLEEP && peer.phase != AWAKE) {
        transmit(peer, MqttSnSession::makePacket(DISCONNECT)); return;
    }
    if (peer.phase == ACTIVE && peer.keepAlive) peer.deadline = simTime() + SimTime(peer.keepAlive * (peer.keepAlive <= 60 ? 1.5 : 1.1));
    if (type == PINGREQ) {
        if (peer.phase == ASLEEP || peer.phase == AWAKE) {
            bool wasAsleep = peer.phase == ASLEEP;
            peer.phase = AWAKE; peer.deadline = -1;
            if (wasAsleep) for (auto& entry : peer.transactions) {
                entry.second.deadline = simTime() + par("retryInterval");
                auto retry = staticPtrCast<MqttSnPacket>(entry.second.packet->dupShared());
                if (retry->getType() == PUBLISH) retry->setFlags(retry->getFlags() | DUPLICATE);
                transmit(peer, retry);
            }
            drive(peer);
        }
        else transmit(peer, MqttSnSession::makePacket(PINGRESP));
        return;
    }
    if (type == REGISTER) {
        if (replayReply(peer, packet)) return;
        auto reply = MqttSnSession::makePacket(REGACK, id);
        int topic = mqtt::MqttTopicFilter::isValidTopic(packet->getTopic()) ? peer.allocateTopic(packet->getTopic()) : 0;
        reply->setTopicId(topic); reply->setReturnCode(topic ? ACCEPTED : CONGESTION); rememberReply(peer, packet, reply); return;
    }
    if (type == SUBSCRIBE || type == UNSUBSCRIBE) {
        if (replayReply(peer, packet)) return;
        for (const auto& entry : peer.pendingSubscriptions) if (MqttSnSession::sameRequest(*packet, *entry.second)) return;
        std::string filter = topicKind(*packet) == NORMAL_TOPIC ? packet->getTopic() : peer.resolveTopic(*packet);
        if (!mqtt::MqttTopicFilter::isValidFilter(filter) || qos(*packet) < 0 || !peer.brokerActive) {
            if (type == SUBSCRIBE) { auto reply = MqttSnSession::makePacket(SUBACK, id); reply->setReturnCode(NOT_SUPPORTED); rememberReply(peer, packet, reply); }
            return;
        }
        int mqttId = peer.mqttSession.allocateIdentifier(); if (!mqttId) return;
        auto request = makeShared<mqtt::MqttPacket>(); request->setType(type == SUBSCRIBE ? mqtt::SUBSCRIBE : mqtt::UNSUBSCRIBE);
        request->setPacketIdentifier(mqttId); request->setFiltersArraySize(1); request->setFilters(0, filter.c_str());
        if (type == SUBSCRIBE) { request->setResultsArraySize(1); request->setResults(0, qos(*packet)); }
        peer.pendingSubscriptions[mqttId] = packet; brokerSend(peer, request); return;
    }
    if (type == PUBLISH) {
        std::string topic = peer.resolveTopic(*packet);
        if (topic.empty() || qos(*packet) < 0) {
            auto reply = MqttSnSession::makePacket(PUBACK, id); reply->setTopicId(packet->getTopicId()); reply->setReturnCode(INVALID_TOPIC); transmit(peer, reply); return;
        }
        if (qos(*packet) != 2 || peer.inboundQos2.insert(id).second) enqueueBroker(peer, packet, topic);
        if (qos(*packet) > 0) {
            auto reply = MqttSnSession::makePacket(qos(*packet) == 1 ? PUBACK : PUBREC, id); reply->setTopicId(packet->getTopicId()); transmit(peer, reply);
        }
        return;
    }
    if (type == PUBREL) { peer.inboundQos2.erase(id); transmit(peer, MqttSnSession::makePacket(PUBCOMP, id)); return; }
    if (type == WILLTOPICUPD || type == WILLMSGUPD) {
        auto reply = MqttSnSession::makePacket(type == WILLTOPICUPD ? WILLTOPICRESP : WILLMSGRESP);
        if (type == WILLTOPICUPD) {
            if (!packet->getWillTopicPresent()) peer.will = nullptr;
            else if (!mqtt::MqttTopicFilter::isValidTopic(packet->getTopic()) || qos(*packet) < 0) reply->setReturnCode(NOT_SUPPORTED);
            else {
                if (!peer.will) peer.will = MqttSnSession::makePacket(PUBLISH);
                peer.will->setTopic(packet->getTopic()); peer.will->setFlags(packet->getFlags() & (0x60 | RETAIN));
            }
        }
        else if (peer.will) MqttSnSession::setPayload(peer.will, MqttSnSession::payload(*packet));
        transmit(peer, reply); return;
    }
    if (type == PUBACK && id == 0) {
        auto context = peer.qosZeroTopics.find(packet->getTopicId());
        if (context == peer.qosZeroTopics.end()) return;
        if (packet->getReturnCode() == CONGESTION) peer.publishAfter = simTime() + par("congestionWait");
        else if (packet->getReturnCode() == INVALID_TOPIC && context->second == NORMAL_TOPIC) peer.topics.erase(packet->getTopicId());
        else if (packet->getReturnCode() == INVALID_TOPIC) EV_WARN << "Aborting publication with an unknown predefined/short topic\n";
        return;
    }
    auto pending = peer.transactions.find(id);
    if (pending == peer.transactions.end()) return;
    auto request = pending->second.packet;
    if (type == REGACK && request->getType() == REGISTER && request->getTopicId() == packet->getTopicId()) {
        if (packet->getReturnCode() != ACCEPTED) {
            peer.excludedTopics.insert(request->getTopic()); peer.topics.erase(request->getTopicId());
        }
        peer.transactions.erase(id); drive(peer); return;
    }
    if (type == PUBREC && request->getType() == PUBLISH && qos(*request) == 2) {
        auto release = MqttSnSession::makePacket(PUBREL, id);
        peer.transactions[id] = {release, peer.phase == ASLEEP ? SimTime::getMaxTime() : simTime() + par("retryInterval"), 0};
        if (peer.phase != ASLEEP) transmit(peer, release);
        return;
    }
    if (type == PUBREC && request->getType() == PUBREL) { if (peer.phase != ASLEEP) transmit(peer, staticPtrCast<MqttSnPacket>(request->dupShared())); return; }
    if (type == PUBCOMP && request->getType() == PUBREL) { peer.transactions.erase(id); drive(peer); return; }
    if (type == PUBACK && request->getType() == PUBLISH && request->getTopicId() == packet->getTopicId()) {
        if (packet->getReturnCode() == ACCEPTED && qos(*request) != 1) return;
        peer.transactions.erase(id);
        if (packet->getReturnCode() != ACCEPTED) {
            auto retry = staticPtrCast<MqttSnPacket>(request->dupShared());
            retry->setTopic(peer.resolveTopic(*request).c_str());
            if (packet->getReturnCode() == INVALID_TOPIC && topicKind(*request) == NORMAL_TOPIC) peer.topics.erase(request->getTopicId());
            if (packet->getReturnCode() == CONGESTION) peer.publishAfter = simTime() + par("congestionWait");
            if (topicKind(*request) == NORMAL_TOPIC || packet->getReturnCode() == CONGESTION) peer.publications.push_front(retry);
            else if (packet->getReturnCode() == INVALID_TOPIC) EV_WARN << "Aborting publication with an unknown predefined/short topic\n";
        }
        drive(peer);
    }
}

void MqttSnGateway::drive(Peer& peer)
{
    if (peer.phase != ACTIVE && peer.phase != AWAKE) return;
    if (!peer.transactions.empty() || simTime() < peer.publishAfter) return;
    while (!peer.publications.empty()) {
        auto packet = peer.publications.front(); std::string topic = packet->getTopic();
        if (topic.size() > 65499 || packet->getPayloadArraySize() > 65498) {
            EV_WARN << "Aborting unrepresentable MQTT-SN downlink publication\n";
            peer.publications.pop_front(); continue;
        }
        if (peer.excludedTopics.count(topic)) { peer.publications.pop_front(); continue; }
        int kind = NORMAL_TOPIC;
        int id = 0;
        for (const auto& binding : peer.predefinedTopics) if (binding.second == topic) { id = binding.first; kind = PREDEFINED_TOPIC; break; }
        if (!id && topic.size() == 2) { id = uint8_t(topic[0]) * 256 + uint8_t(topic[1]); kind = SHORT_TOPIC; }
        if (!id) id = peer.findTopic(topic);
        if (!id) {
            id = peer.allocateTopic(topic);
            if (!id) return;
            auto registration = MqttSnSession::makePacket(REGISTER, peer.allocateMessageId());
            registration->setTopicId(id); registration->setTopic(topic.c_str()); transmit(peer, registration, true); return;
        }
        peer.publications.pop_front();
        auto wire = staticPtrCast<MqttSnPacket>(packet->dupShared()); wire->setTopic(""); wire->setTopicId(id);
        wire->setFlags((wire->getFlags() & ~3) | kind); wire->setMessageId(qos(*wire) ? peer.allocateMessageId() : 0);
        transmit(peer, wire, qos(*wire) > 0);
        if (qos(*wire) > 0) return;
    }
    if (peer.phase == AWAKE) {
        transmit(peer, MqttSnSession::makePacket(PINGRESP)); peer.phase = ASLEEP;
        peer.deadline = simTime() + SimTime(peer.sleepDuration * (peer.sleepDuration <= 60 ? 1.5 : 1.1));
    }
}

void MqttSnGateway::restoreSubscriptions(Peer& peer)
{
    while (!peer.restoreQueue.empty()) {
        int id = peer.mqttSession.allocateIdentifier();
        if (!id) return;
        auto subscription = peer.restoreQueue.front(); peer.restoreQueue.pop_front();
        auto request = makeShared<mqtt::MqttPacket>(); request->setType(mqtt::SUBSCRIBE); request->setPacketIdentifier(id);
        request->setFiltersArraySize(1); request->setFilters(0, subscription.first.c_str());
        request->setResultsArraySize(1); request->setResults(0, subscription.second);
        peer.restoring[id] = subscription; brokerSend(peer, request);
    }
    if (peer.restoring.empty()) for (auto outgoing : peer.mqttSession.flush()) brokerSend(peer, outgoing);
}

void MqttSnGateway::brokerReceive(Peer& peer, const Ptr<mqtt::MqttPacket>& packet)
{
    int type = packet->getType(), id = packet->getPacketIdentifier();
    if (!peer.brokerActive) {
        if (type != mqtt::CONNACK || (peer.cleanSession && packet->getSessionPresent())) throw cRuntimeError("Invalid upstream MQTT CONNACK");
        if (packet->getReturnCode()) {
            failBroker(peer, packet->getReturnCode() == 3 ? CONGESTION : NOT_SUPPORTED); return;
        }
        peer.brokerActive = true; peer.brokerDeadline = -1;
        if (!packet->getSessionPresent()) {
            peer.mqttSession.incomingQos2.clear();
            for (const auto& subscription : peer.subscriptions) peer.restoreQueue.push_back(subscription);
        }
        for (auto outgoing : peer.mqttSession.retry()) brokerSend(peer, outgoing);
        restoreSubscriptions(peer);
        if (&peer != &shared) {
            peer.phase = ACTIVE;
            peer.deadline = peer.keepAlive ? simTime() + SimTime(peer.keepAlive * (peer.keepAlive <= 60 ? 1.5 : 1.1)) : SimTime(-1);
            transmit(peer, MqttSnSession::makePacket(CONNACK));
            for (auto& entry : peer.transactions) entry.second.deadline = simTime() + par("retryInterval");
            emit(registerSignal("sessionStateChanged"), long(ACTIVE)); drive(peer);
        }
        return;
    }
    if (type == mqtt::PINGRESP) { peer.brokerPingDeadline = -1; return; }
    if (type == mqtt::SUBACK || type == mqtt::UNSUBACK) {
        auto restore = peer.restoring.find(id);
        if (restore != peer.restoring.end()) {
            if (type != mqtt::SUBACK || packet->getResultsArraySize() != 1 || (packet->getResults(0) != 128 && packet->getResults(0) > restore->second.second))
                throw cRuntimeError("Invalid MQTT restoration acknowledgement");
            peer.restoring.erase(restore); peer.mqttSession.releaseIdentifier(id); restoreSubscriptions(peer); return;
        }
        auto pending = peer.pendingSubscriptions.find(id);
        if (pending == peer.pendingSubscriptions.end()) throw cRuntimeError("Unmatched MQTT subscription acknowledgement");
        auto original = staticPtrCast<MqttSnPacket>(pending->second->dupShared());
        if ((original->getType() == SUBSCRIBE) != (type == mqtt::SUBACK) ||
                (type == mqtt::SUBACK && (packet->getResultsArraySize() != 1 || (packet->getResults(0) != 128 && packet->getResults(0) > qos(*original)))))
            throw cRuntimeError("Invalid upstream MQTT subscription result");
        std::string filter = topicKind(*original) == NORMAL_TOPIC ? original->getTopic() : peer.resolveTopic(*original);
        auto reply = MqttSnSession::makePacket(type == mqtt::SUBACK ? SUBACK : UNSUBACK, original->getMessageId());
        if (type == mqtt::SUBACK) {
            if (packet->getResults(0) == 128) reply->setReturnCode(NOT_SUPPORTED);
            else {
                // A new subscription makes an older UNSUBSCRIBE replay obsolete.
                peer.replies.erase(UNSUBSCRIBE);
                peer.subscriptions[filter] = packet->getResults(0); reply->setFlags(qosFlags(packet->getResults(0)));
                if (filter.find_first_of("+#") == std::string::npos && topicKind(*original) == NORMAL_TOPIC) {
                    int topicId = peer.allocateTopic(filter); reply->setTopicId(topicId);
                    if (!topicId) reply->setReturnCode(CONGESTION);
                }
                else if (topicKind(*original) == PREDEFINED_TOPIC) reply->setTopicId(original->getTopicId());
                for (auto it = peer.excludedTopics.begin(); it != peer.excludedTopics.end();) {
                    if (mqtt::MqttTopicFilter::matches(filter, *it)) it = peer.excludedTopics.erase(it); else ++it;
                }
            }
        }
        else peer.subscriptions.erase(filter);
        peer.pendingSubscriptions.erase(pending); peer.mqttSession.releaseIdentifier(id); restoreSubscriptions(peer); rememberReply(peer, original, reply); return;
    }
    if (type == mqtt::PUBLISH) {
        if (!mqtt::MqttTopicFilter::isValidTopic(packet->getTopic())) throw cRuntimeError("Invalid upstream MQTT topic");
        if (peer.mqttSession.acceptPublication(packet)) {
            auto publication = MqttSnSession::makePacket(PUBLISH); publication->setTopic(packet->getTopic());
            publication->setFlags(qosFlags(packet->getQos()) | (packet->getRetain() ? RETAIN : 0));
            publication->setPayloadArraySize(packet->getPayloadArraySize());
            for (size_t i = 0; i < packet->getPayloadArraySize(); i++) publication->setPayload(i, packet->getPayload(i));
            if (&peer != &shared) peer.publications.push_back(publication);
            drive(peer);
        }
        if (packet->getQos()) { auto reply = makeShared<mqtt::MqttPacket>(); reply->setType(packet->getQos() == 1 ? mqtt::PUBACK : mqtt::PUBREC); reply->setPacketIdentifier(id); brokerSend(peer, reply); }
        return;
    }
    if (type == mqtt::PUBREL) {
        peer.mqttSession.incomingQos2.erase(id); auto reply = makeShared<mqtt::MqttPacket>(); reply->setType(mqtt::PUBCOMP); reply->setPacketIdentifier(id); brokerSend(peer, reply); return;
    }
    if (type == mqtt::PUBACK || type == mqtt::PUBREC || type == mqtt::PUBCOMP) {
        if (auto reply = peer.mqttSession.acknowledge(packet)) brokerSend(peer, reply);
        restoreSubscriptions(peer);
        return;
    }
    throw cRuntimeError("Unexpected upstream MQTT message");
}

void MqttSnGateway::socketDataArrived(UdpSocket *, Packet *message)
{
    if (message->getDataLength() > B(65507)) { delete message; return; }
    auto source = message->getTag<L3AddressInd>()->getSrcAddress(); int port = message->getTag<L4PortInd>()->getSrcPort();
    try {
        auto packet = MqttSnPacketSerializer::decode(message->peekDataAsBytes()->getBytes());
        emit(registerSignal("packetReceivedFromLower"), message);
        if (packet->getType() == SEARCHGW) {
            if (packet->getRadius() == 1 && shared.brokerActive) {
                auto info = MqttSnSession::makePacket(GWINFO); info->setGatewayId(par("gatewayId"));
                sendTo(info, L3AddressResolver().resolve(par("broadcastAddress")), par("localPort"));
            }
        }
        else if (packet->getType() == CONNECT) acceptConnect(packet, source, port);
        else if (packet->getType() == PUBLISH && qos(*packet) == -1) {
            if (topicKind(*packet) == PREDEFINED_TOPIC || topicKind(*packet) == SHORT_TOPIC) {
                auto topic = shared.resolveTopic(*packet); if (!topic.empty()) enqueueBroker(shared, packet, topic);
            }
        }
        else {
            Peer *peer = nullptr;
            for (auto& entry : peers) if (entry.second.address == source && entry.second.port == port) { peer = &entry.second; break; }
            if (packet->getType() == PINGREQ && *packet->getClientIdentifier()) {
                auto found = peers.find(packet->getClientIdentifier());
                if (found != peers.end() && found->second.address == source && found->second.port == port) peer = &found->second;
                else peer = nullptr;
            }
            if (peer) receiveControlPacket(*peer, packet);
            else if (packet->getType() != ADVERTISE && packet->getType() != GWINFO && packet->getType() != DISCONNECT)
                sendTo(MqttSnSession::makePacket(DISCONNECT), source, port);
        }
    }
    catch (cRuntimeError& error) { EV_WARN << error.what() << endl; }
    delete message; scheduleTimer();
}

void MqttSnGateway::socketDataArrived(TcpSocket *connection, Packet *message, bool)
{
    auto peer = findPeer(connection);
    if (!peer) { delete message; return; }
    connection->getReadBuffer()->push(message->peekData()); delete message;
    try {
        while (auto packet = mqtt::MqttConnection::extract(*connection->getReadBuffer())) {
            Packet observation("mqttReceived", packet); observation.addTag<PacketProtocolTag>()->setProtocol(&mqtt::mqttProtocol);
            emit(registerSignal("packetReceivedFromLower"), &observation); brokerReceive(*peer, packet);
            if (peer->socketId != connection->getSocketId()) break;
        }
    }
    catch (cRuntimeError& error) { EV_WARN << error.what() << endl; failBroker(*peer); }
    scheduleTimer();
}

void MqttSnGateway::socketPeerClosed(TcpSocket *connection) { if (auto peer = findPeer(connection)) failBroker(*peer); else if (connection->isOpen()) connection->close(); }
void MqttSnGateway::socketClosed(TcpSocket *connection) { if (auto peer = findPeer(connection)) failBroker(*peer); }
void MqttSnGateway::socketFailure(TcpSocket *connection, int) { if (auto peer = findPeer(connection)) failBroker(*peer); }
void MqttSnGateway::socketErrorArrived(UdpSocket *, Indication *indication) { delete indication; }
void MqttSnGateway::socketClosed(UdpSocket *) { bound = false; }
void MqttSnGateway::handleMessageWhenDown(cMessage *message) { delete message; }

void MqttSnGateway::reapSockets()
{
    std::vector<TcpSocket *> terminal;
    for (const auto& entry : brokerSockets.getMap()) {
        auto connection = check_and_cast<TcpSocket *>(entry.second);
        if (connection->getState() == TcpSocket::CLOSED || connection->getState() == TcpSocket::SOCKERROR) terminal.push_back(connection);
    }
    for (auto connection : terminal) { brokerSockets.removeSocket(connection); delete connection; }
}

void MqttSnGateway::scheduleTimer()
{
    cancelEvent(timer); if (!bound) return;
    simtime_t next = SimTime::getMaxTime();
    if (!shared.brokerActive && shared.socketId < 0) next = sharedReconnectAt;
    if (shared.brokerActive && par("advertise").boolValue()) next = std::min(next, advertiseAt);
    auto consider = [&](const Peer& peer) {
        for (simtime_t value : {peer.deadline, peer.brokerDeadline, peer.brokerPingDeadline}) if (value >= SIMTIME_ZERO) next = std::min(next, value);
        if (peer.brokerActive && par("brokerKeepAlive").intValue() && peer.brokerPingDeadline < SIMTIME_ZERO)
            next = std::min(next, peer.brokerLastSent + SimTime(par("brokerKeepAlive")));
        for (const auto& transaction : peer.transactions) next = std::min(next, transaction.second.deadline);
        if ((peer.phase == ACTIVE || peer.phase == AWAKE) && peer.transactions.empty() && !peer.publications.empty() && peer.publishAfter > simTime())
            next = std::min(next, peer.publishAfter);
    };
    consider(shared); for (const auto& entry : peers) consider(entry.second);
    if (next != SimTime::getMaxTime()) scheduleAt(std::max(simTime(), next), timer);
}

void MqttSnGateway::handleMessageWhenUp(cMessage *message)
{
    if (message != timer) {
        if (auto connection = check_and_cast_nullable<TcpSocket *>(brokerSockets.findSocketFor(message))) connection->processMessage(message);
        else if (socket.belongsToSocket(message)) socket.processMessage(message);
        else delete message;
        reapSockets(); scheduleTimer(); return;
    }
    if (!shared.brokerActive && shared.socketId < 0 && sharedReconnectAt <= simTime()) openBroker(shared);
    if (shared.brokerActive && par("advertise").boolValue() && advertiseAt <= simTime()) {
        auto packet = MqttSnSession::makePacket(ADVERTISE); packet->setGatewayId(par("gatewayId")); packet->setDuration(par("advertisementInterval"));
        sendTo(packet, L3AddressResolver().resolve(par("broadcastAddress")), par("localPort")); advertiseAt = simTime() + SimTime(par("advertisementInterval"));
    }
    auto expire = [&](Peer& peer) {
        if ((peer.brokerDeadline >= SIMTIME_ZERO && peer.brokerDeadline <= simTime()) || (peer.brokerPingDeadline >= SIMTIME_ZERO && peer.brokerPingDeadline <= simTime())) { failBroker(peer); return; }
        if (peer.brokerActive && par("brokerKeepAlive").intValue() && peer.brokerPingDeadline < SIMTIME_ZERO && simTime() >= peer.brokerLastSent + SimTime(par("brokerKeepAlive"))) {
            auto ping = makeShared<mqtt::MqttPacket>(); ping->setType(mqtt::PINGREQ); brokerSend(peer, ping); peer.brokerPingDeadline = simTime() + par("brokerPingTimeout");
        }
        if (&peer == &shared) return;
        if (peer.deadline >= SIMTIME_ZERO && peer.deadline <= simTime()) { disconnectPeer(peer, true); return; }
        for (auto& entry : peer.transactions) {
            if (entry.second.deadline > simTime()) continue;
            if (entry.second.retries >= par("retryCount").intValue()) { disconnectPeer(peer, peer.phase != WILL_TOPIC && peer.phase != WILL_MESSAGE && peer.phase != CONNECTING); return; }
            auto retry = staticPtrCast<MqttSnPacket>(entry.second.packet->dupShared()); entry.second.retries++; entry.second.deadline = simTime() + par("retryInterval");
            if (retry->getType() == PUBLISH) retry->setFlags(retry->getFlags() | DUPLICATE);
            transmit(peer, retry);
        }
        drive(peer);
    };
    expire(shared); for (auto& entry : peers) expire(entry.second);
    reapSockets(); scheduleTimer();
}

void MqttSnGateway::handleStopOperation(LifecycleOperation *)
{
    cancelEvent(timer);
    for (auto& entry : peers) {
        auto& peer = entry.second;
        if (peer.phase != DISCONNECTED && peer.phase != LOST) transmit(peer, MqttSnSession::makePacket(DISCONNECT));
        disconnectPeer(peer, false);
    }
    closeBroker(shared); socket.close(); bound = false; cancelEvent(timer);
}

void MqttSnGateway::handleCrashOperation(LifecycleOperation *)
{
    cancelEvent(timer); socket.destroy(); bound = false;
    for (const auto& entry : brokerSockets.getMap()) entry.second->destroy();
    brokerSockets.deleteSockets(); peers.clear(); shared.mqttSession.clearExchanges(); shared.socketId = -1; shared.brokerActive = false;
    shared.brokerDeadline = shared.brokerPingDeadline = -1;
}

void MqttSnGateway::handleActiveOperationTimeout(cMessage *) { handleCrashOperation(nullptr); finishActiveOperation(); }

} // namespace inet::mqttsn
