// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/applications/mqttsn/MqttSnClient.h"
#include "inet/applications/mqttsn/MqttSnPacketSerializer.h"
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
#include "inet/common/packet/chunk/BytesChunk.h"

namespace inet::mqttsn {
Define_Module(MqttSnClient);

MqttSnClient::~MqttSnClient() { cancelAndDelete(timer); }

void MqttSnClient::initialize(int stage)
{
    ApplicationBase::initialize(stage);
    if (stage != INITSTAGE_LOCAL) return;
    timer = new cMessage("mqttSnDeadlines");
    if (par("retryInterval").doubleValue() <= 0 || par("retryCount").intValue() < 0 ||
            par("congestionWait").doubleValue() <= 0 || par("searchResponseTimeout").doubleValue() <= 0 ||
            par("searchRetryMax").doubleValue() < par("searchResponseTimeout").doubleValue() ||
            par("gatewayInfoLifetime").doubleValue() <= 0 || par("searchDelay").doubleValue() < 0 ||
            par("gatewayInfoDelay").doubleValue() < 0 || par("missedAdvertisements").intValue() < 1 ||
            par("keepAlive").intValue() < 0 || par("keepAlive").intValue() > 65535 ||
            par("willQos").intValue() < 0 || par("willQos").intValue() > 2 ||
            par("stopOperationTimeout").doubleValue() <= 0 || par("startTime").doubleValue() < 0)
        throw cRuntimeError("Invalid MQTT-SN client duration or counter");
    MqttSnSession::readPredefined(par("predefinedTopics").xmlValue(), session.predefinedTopics);
    readWorkload();
}

void MqttSnClient::readWorkload()
{
    simtime_t previous = SIMTIME_ZERO;
    for (auto element : par("workload").xmlValue()->getChildren()) {
        auto attribute = [&](const char *key, const char *fallback) { const char *value = element->getAttribute(key); return value ? value : fallback; };
        auto integer = [&](const char *key, const char *fallback) {
            const char *text = attribute(key, fallback); char *end = nullptr;
            long value = strtol(text, &end, 10);
            if (end == text || *end || value < -1 || value > 65535) throw cRuntimeError("Invalid MQTT-SN workload integer %s", key);
            return int(value);
        };
        std::string name = element->getTagName();
        simtime_t at = SimTime::parse(attribute("at", "0s"));
        if (at < previous) throw cRuntimeError("MQTT-SN workload offsets must be nonnegative and ordered");
        previous = at;
        int type = name == "publish" ? PUBLISH : name == "subscribe" ? SUBSCRIBE : name == "unsubscribe" ? UNSUBSCRIBE :
            name == "sleep" ? DISCONNECT : name == "poll" ? PINGREQ : name == "willTopic" ? WILLTOPICUPD : name == "willMessage" ? WILLMSGUPD : -1;
        if (type < 0) throw cRuntimeError("Unknown MQTT-SN workload action %s", name.c_str());
        auto packet = MqttSnSession::makePacket(type);
        std::string topic = attribute(type == PUBLISH ? "topic" : "filter", "");
        if (type == WILLTOPICUPD) topic = attribute("topic", "");
        packet->setTopic(topic.c_str());
        int level = integer("qos", "0");
        int kind = integer("topicKind", "0");
        if (level < -1 || level > 2 || kind < 0 || kind > 2) throw cRuntimeError("Invalid MQTT-SN workload QoS/topic kind");
        if ((type == PUBLISH && level == -1 && kind == NORMAL_TOPIC) ||
                ((type == SUBSCRIBE || type == WILLTOPICUPD) && level < 0))
            throw cRuntimeError("QoS -1 requires a predefined/short PUBLISH topic");
        packet->setFlags(qosFlags(level) | kind | (std::string(attribute("retain", "false")) == "true" ? RETAIN : 0));
        if (kind == PREDEFINED_TOPIC) {
            int id = integer("topicId", "0");
            if (!session.predefinedTopics.count(id)) throw cRuntimeError("Unknown predefined workload topic");
            packet->setTopicId(id);
        }
        if (kind == SHORT_TOPIC) {
            if (topic.size() != 2 || !mqtt::MqttTopicFilter::isValidTopic(topic)) throw cRuntimeError("Short topic must contain two valid octets");
            packet->setTopicId(uint8_t(topic[0]) * 256 + uint8_t(topic[1]));
        }
        if ((type == PUBLISH && kind == NORMAL_TOPIC && !mqtt::MqttTopicFilter::isValidTopic(topic)) ||
                ((type == SUBSCRIBE || type == UNSUBSCRIBE) && kind == NORMAL_TOPIC && !mqtt::MqttTopicFilter::isValidFilter(topic)))
            throw cRuntimeError("Invalid MQTT-SN workload topic/filter");
        if (type == DISCONNECT) {
            simtime_t duration = SimTime::parse(attribute("duration", "0s"));
            if (duration < SIMTIME_ZERO || duration > SimTime(65535) || duration != SimTime(int(duration.dbl())))
                throw cRuntimeError("MQTT-SN sleep duration must be whole seconds in 0..65535");
            packet->setDurationPresent(true); packet->setDuration(duration.dbl());
        }
        packet->setWillTopicPresent(!topic.empty());
        MqttSnSession::setPayload(packet, MqttSnSession::parseHex(attribute("payload", "")));
        if (type == PUBLISH && packet->getPayloadArraySize() > 65498)
            throw cRuntimeError("MQTT-SN workload publication exceeds UDP profile limit");
        if (par("connectionless").boolValue() && (type != PUBLISH || level != -1 || kind == NORMAL_TOPIC))
            throw cRuntimeError("Connectionless MQTT-SN workload requires QoS -1 and predefined/short topics");
        actions.emplace_back(at, packet);
    }
}

void MqttSnClient::changePhase(Phase value)
{
    if (phase != value) { phase = value; emit(registerSignal("sessionStateChanged"), long(value)); }
}

void MqttSnClient::handleStartOperation(LifecycleOperation *)
{
    socket.setOutputGate(gate("socketOut")); socket.setCallback(this);
    const char *local = par("localAddress");
    socket.bind(*local ? L3AddressResolver().resolve(local) : L3Address(), par("localPort"));
    socket.setBroadcast(true); bound = true;
    const char *configured = par("gatewayAddress");
    gateway = *configured ? L3AddressResolver().resolve(configured) : L3Address();
    if (par("connectionless") && gateway.isUnspecified()) throw cRuntimeError("Connectionless MQTT-SN needs a configured gateway");
    reconnectAt = std::max(simTime(), SimTime(par("startTime")));
    if (par("connectionless")) { if (origin < SIMTIME_ZERO) origin = reconnectAt; changePhase(ACTIVE); }
    else changePhase(DISCONNECTED);
    scheduleTimer();
}

void MqttSnClient::sendTo(const Ptr<MqttSnPacket>& packet, const L3Address& address, int port)
{
    MqttSnPacketSerializer::setLength(packet);
    if (packet->getChunkLength() > B(65507)) { EV_WARN << "MQTT-SN UDP message exceeds profile payload limit\n"; return; }
    auto message = new Packet("mqttsn", packet);
    message->addTag<PacketProtocolTag>()->setProtocol(&mqttSnProtocol);
    if (packet->getType() == SEARCHGW || packet->getType() == GWINFO) {
        auto table = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        auto interface = table->findInterfaceByName(par("discoveryInterface"));
        if (!interface) { delete message; throw cRuntimeError("Unknown MQTT-SN discovery interface"); }
        message->addTag<InterfaceReq>()->setInterfaceId(interface->getInterfaceId());
        message->addTag<HopLimitReq>()->setHopLimit(1);
    }
    emit(registerSignal("packetSentToLower"), message);
    socket.sendTo(message, address, port);
}

void MqttSnClient::transmit(const Ptr<MqttSnPacket>& packet, bool track)
{
    if (packet->getType() == PUBLISH && qos(*packet) == 0) session.qosZeroTopics[packet->getTopicId()] = topicKind(*packet);
    if (track) {
        int key = packet->getMessageId() ? packet->getMessageId() : -packet->getType();
        session.transactions[key] = {packet, simTime() + par("retryInterval"), 0};
    }
    lastSent = simTime();
    sendTo(packet, gateway, par("gatewayPort"));
}

void MqttSnClient::connectGateway()
{
    // MQTT-SN v1.2 (2013), 6.1 and 6.13: try alternative gateways before searching again.
    if (gateway.isUnspecified()) {
        for (const auto& candidate : candidates) {
            if (candidate.second.expiry <= simTime() || attemptedGateways.count(candidate.first)) continue;
            if (selectedGatewayId >= 0 && selectedGatewayId != candidate.first) {
                resetSession = true; session.topics.clear(); session.clearExchanges();
            }
            selectedGatewayId = candidate.first; attemptedGateways.insert(candidate.first); gateway = candidate.second.address;
            break;
        }
        if (gateway.isUnspecified()) {
            if (searchAt < SIMTIME_ZERO && searchResponseAt < SIMTIME_ZERO)
                searchAt = simTime() + uniform(0, par("searchDelay").doubleValue());
            reconnectAt = SimTime::getMaxTime();
            return;
        }
    }
    // MQTT-SN v1.2 (2013), 6.2–6.3. Gateway changes force clean setup under our UDP profile.
    effectiveClean = par("cleanSession").boolValue() || resetSession;
    if (effectiveClean) { session.clearExchanges(); session.topics.clear(); }
    auto packet = MqttSnSession::makePacket(CONNECT);
    packet->setFlags((effectiveClean ? CLEAN_SESSION : 0) | (*par("willTopic").stringValue() ? WILL : 0));
    packet->setClientIdentifier(par("clientIdentifier")); packet->setDuration(par("keepAlive"));
    changePhase(CONNECTING); transmit(packet, true);
}

void MqttSnClient::loseGateway()
{
    EV_WARN << "MQTT-SN gateway relationship ended; unfinished outcomes may be uncertain\n";
    if (effectiveClean) session.clearExchanges();
    else {
        for (auto it = session.transactions.begin(); it != session.transactions.end();) {
            if (it->second.packet->getType() != PUBLISH && it->second.packet->getType() != PUBREL) it = session.transactions.erase(it);
            else { it->second.deadline = SimTime::getMaxTime(); ++it; }
        }
    }
    reconciliation.clear(); reconcileOnConnect = true;
    if (selectedGatewayId >= 0) attemptedGateways.insert(selectedGatewayId);
    changePhase(DISCONNECTED);
    if (!*par("gatewayAddress").stringValue()) gateway = L3Address();
    reconnectAt = simTime() + par("retryInterval");
}

void MqttSnClient::complete(int key) { session.transactions.erase(key); }

void MqttSnClient::deliver(const Ptr<MqttSnPacket>& packet)
{
    auto message = new Packet("mqttSnDelivered", makeShared<BytesChunk>(MqttSnSession::payload(*packet)));
    emit(registerSignal("packetSentToUpper"), message); delete message;
}

void MqttSnClient::receiveControlPacket(const Ptr<MqttSnPacket>& packet, const L3Address& source, int port)
{
    int type = packet->getType(), id = packet->getMessageId();
    if (type == ADVERTISE || type == SEARCHGW || type == GWINFO) { receiveDiscovery(packet, source); return; }
    if (source != gateway || port != par("gatewayPort").intValue()) return;
    if (type == CONNACK && phase == CONNECTING) {
        complete(-CONNECT);
        if (packet->getReturnCode() != ACCEPTED) { loseGateway(); return; }
        if (resetSession || effectiveClean || reconcileOnConnect) {
            reconciliation.clear();
            for (const auto& desired : session.subscriptions) {
                auto request = MqttSnSession::makePacket(SUBSCRIBE); request->setTopic(desired.first.c_str()); request->setFlags(qosFlags(desired.second));
                reconciliation.push_back(request);
            }
        }
        reconcileOnConnect = false; resetSession = false; attemptedGateways.clear();
        if (origin < SIMTIME_ZERO) origin = simTime();
        changePhase(ACTIVE); lastSent = simTime();
        for (auto& entry : session.transactions) {
            entry.second.deadline = simTime() + par("retryInterval"); entry.second.retries = 0;
            auto retry = staticPtrCast<MqttSnPacket>(entry.second.packet->dupShared());
            if (retry->getType() == PUBLISH) retry->setFlags(retry->getFlags() | DUPLICATE);
            transmit(retry);
        }
        return;
    }
    if ((type == WILLTOPICREQ || type == WILLMSGREQ) && phase == CONNECTING) {
        // MQTT-SN v1.2 (2013), 6.2: the gateway prompts separately for Will topic and message.
        auto reply = MqttSnSession::makePacket(type == WILLTOPICREQ ? WILLTOPIC : WILLMSG);
        if (type == WILLTOPICREQ) {
            reply->setWillTopicPresent(*par("willTopic").stringValue()); reply->setTopic(par("willTopic"));
            reply->setFlags(qosFlags(par("willQos")) | (par("willRetain").boolValue() ? RETAIN : 0));
        }
        else MqttSnSession::setPayload(reply, MqttSnSession::parseHex(par("willPayload")));
        transmit(reply); return;
    }
    if (type == DISCONNECT) {
        // MQTT-SN v1.2 (2013), 6.12 and 6.14: a sleep acknowledgement omits the duration.
        if (phase == CLOSING) { complete(-DISCONNECT); socket.close(); return; }
        auto pending = session.transactions.find(-DISCONNECT);
        if (pending != session.transactions.end() && pending->second.packet->getDurationPresent()) {
            complete(-DISCONNECT); changePhase(ASLEEP); return;
        }
        loseGateway(); return;
    }
    if (phase != ACTIVE && phase != ASLEEP && phase != AWAKE) return;
    if (phase == AWAKE && type != PINGRESP && session.transactions.count(-PINGREQ))
        session.transactions.at(-PINGREQ).deadline = simTime() + par("retryInterval");
    if (type == REGISTER) {
        // MQTT-SN v1.2 (2013), 6.5 and 6.10: bind the concrete downlink topic before publishing.
        auto reply = MqttSnSession::makePacket(REGACK, id); reply->setTopicId(packet->getTopicId());
        bool valid = packet->getTopicId() > 0 && packet->getTopicId() < 65535 && mqtt::MqttTopicFilter::isValidTopic(packet->getTopic());
        if (valid) session.topics[packet->getTopicId()] = packet->getTopic();
        reply->setReturnCode(valid ? ACCEPTED : INVALID_TOPIC); transmit(reply); return;
    }
    if (type == PUBLISH) {
        // MQTT-SN v1.2 (2013), 6.6–6.7 and 6.10; inherited MQTT QoS 2 Method B retains the id until PUBREL.
        auto topic = session.resolveTopic(*packet);
        if (topic.empty() || qos(*packet) < 0) {
            auto reply = MqttSnSession::makePacket(PUBACK, id); reply->setTopicId(packet->getTopicId()); reply->setReturnCode(INVALID_TOPIC); transmit(reply); return;
        }
        if (qos(*packet) != 2 || session.inboundQos2.insert(id).second) deliver(packet);
        if (qos(*packet) > 0) {
            auto reply = MqttSnSession::makePacket(qos(*packet) == 1 ? PUBACK : PUBREC, id);
            reply->setTopicId(packet->getTopicId()); transmit(reply);
        }
        return;
    }
    if (type == PUBREL) { session.inboundQos2.erase(id); transmit(MqttSnSession::makePacket(PUBCOMP, id)); return; }
    if (type == PINGREQ) { transmit(MqttSnSession::makePacket(PINGRESP)); return; }
    // MQTT-SN v1.2 (2013), 6.11 and 6.14: PINGRESP also completes a sleeping client's drain.
    if (type == PINGRESP) {
        if (session.transactions.count(-PINGREQ)) { complete(-PINGREQ); if (phase == AWAKE) changePhase(ASLEEP); }
        return;
    }
    if (type == WILLTOPICRESP || type == WILLMSGRESP) { complete(-(type == WILLTOPICRESP ? WILLTOPICUPD : WILLMSGUPD)); return; }
    if (type == PUBACK && id == 0) {
        // MQTT-SN v1.2 (2013), 6.6–6.7: error PUBACK applies even to QoS 0, without an ACK transaction.
        auto context = session.qosZeroTopics.find(packet->getTopicId());
        if (context == session.qosZeroTopics.end()) return;
        if (packet->getReturnCode() == CONGESTION) session.publishAfter = simTime() + par("congestionWait");
        else if (packet->getReturnCode() == INVALID_TOPIC && context->second == NORMAL_TOPIC) session.topics.erase(packet->getTopicId());
        else if (packet->getReturnCode() == INVALID_TOPIC) EV_WARN << "Aborting publication with an unknown predefined/short topic\n";
        return;
    }
    auto pending = session.transactions.find(id);
    if (pending == session.transactions.end()) return;
    auto request = pending->second.packet;
    if (type == REGACK && request->getType() == REGISTER) {
        // MQTT-SN v1.2 (2013), 6.5 and 6.9: match registration/subscription replies by message id.
        if (packet->getReturnCode() == ACCEPTED && packet->getTopicId() > 0 && packet->getTopicId() < 65535)
            session.topics[packet->getTopicId()] = request->getTopic();
        else session.publishAfter = simTime() + par("congestionWait");
        complete(id); return;
    }
    if ((type == SUBACK && request->getType() == SUBSCRIBE) || (type == UNSUBACK && request->getType() == UNSUBSCRIBE)) {
        if (type == SUBACK && packet->getReturnCode() == ACCEPTED) {
            if (qos(*packet) < 0 || qos(*packet) > qos(*request)) { loseGateway(); return; }
            auto filter = topicKind(*request) == NORMAL_TOPIC ? std::string(request->getTopic()) : session.resolveTopic(*request);
            session.subscriptions[filter] = qos(*packet);
            if (topicKind(*request) == NORMAL_TOPIC && packet->getTopicId() > 0 && packet->getTopicId() < 65535)
                session.topics[packet->getTopicId()] = request->getTopic();
        }
        else if (type == UNSUBACK) session.subscriptions.erase(request->getTopic());
        if (type == SUBACK && packet->getReturnCode() == CONGESTION) session.publishAfter = simTime() + par("congestionWait");
        complete(id); return;
    }
    if (type == PUBREC && request->getType() == PUBLISH && qos(*request) == 2) {
        // MQTT-SN v1.2 (2013), 5.4.14 and 6.6: after PUBREC retain PUBREL, never the old PUBLISH.
        transmit(MqttSnSession::makePacket(PUBREL, id), true); return;
    }
    if (type == PUBREC && request->getType() == PUBREL) { transmit(staticPtrCast<MqttSnPacket>(request->dupShared())); return; }
    if (type == PUBCOMP && request->getType() == PUBREL) { complete(id); return; }
    if (type == PUBACK && request->getType() == PUBLISH && packet->getTopicId() == request->getTopicId()) {
        if (packet->getReturnCode() == ACCEPTED) { if (qos(*request) == 1) complete(id); }
        else {
            auto retry = staticPtrCast<MqttSnPacket>(request->dupShared());
            retry->setTopic(session.resolveTopic(*request).c_str());
            complete(id);
            if (packet->getReturnCode() == INVALID_TOPIC && topicKind(*request) == NORMAL_TOPIC) {
                auto name = session.resolveTopic(*request); session.topics.erase(request->getTopicId()); retry->setTopic(name.c_str());
                session.publications.push_front(retry);
            }
            else if (packet->getReturnCode() == CONGESTION) {
                session.publishAfter = simTime() + par("congestionWait"); session.publications.push_front(retry);
            }
            else if (packet->getReturnCode() == INVALID_TOPIC) EV_WARN << "Aborting publication with an unknown predefined/short topic\n";
        }
    }
}

void MqttSnClient::receiveDiscovery(const Ptr<MqttSnPacket>& packet, const L3Address& source)
{
    // MQTT-SN v1.2 (2013), 6.1: peer-search suppression, delayed GWINFO and candidate lifetimes.
    if (*par("gatewayAddress").stringValue()) return;
    if (packet->getType() == SEARCHGW && packet->getRadius() == 1) {
        if (searchAt >= SIMTIME_ZERO) { searchAt = -1; searchResponseAt = simTime() + par("searchResponseTimeout"); }
        if (!candidates.empty()) gatewayInfoAt = simTime() + uniform(0, par("gatewayInfoDelay").doubleValue());
        return;
    }
    if (packet->getType() == ADVERTISE || packet->getType() == GWINFO) {
        L3Address address = source;
        if (packet->getType() == GWINFO && packet->getGatewayAddressArraySize()) {
            // IPv4 length/address encoding is the local UDP profile in plan/pending/mqtt-sn.md.
            if (packet->getGatewayAddressArraySize() != 5 || packet->getGatewayAddress(0) != 4) return;
            uint32_t value = 0;
            for (int i = 1; i <= 4; i++) value = value * 256 + packet->getGatewayAddress(i);
            address = L3Address(Ipv4Address(value));
        }
        auto found = candidates.find(packet->getGatewayId());
        if (found != candidates.end() && found->second.expiry > simTime() && found->second.address != address) return;
        simtime_t lifetime = packet->getType() == ADVERTISE ? SimTime(packet->getDuration() * par("missedAdvertisements").intValue()) : SimTime(par("gatewayInfoLifetime"));
        candidates[packet->getGatewayId()] = {address, simTime() + lifetime};
        gatewayInfoAt = -1; searchAt = -1; searchResponseAt = -1; searchBackoff = par("searchResponseTimeout");
        if (phase == DISCONNECTED) reconnectAt = std::max(simTime(), SimTime(par("startTime")));
    }
}

void MqttSnClient::drive()
{
    if (phase == DISCONNECTED && simTime() >= reconnectAt) connectGateway();
    if (phase == ACTIVE && session.transactions.empty() && !reconciliation.empty()) {
        auto request = reconciliation.front(); reconciliation.pop_front(); request->setMessageId(session.allocateMessageId());
        transmit(request, true); return;
    }
    if (phase == ACTIVE && !session.publications.empty() && session.transactions.empty() && simTime() >= session.publishAfter) {
        // MQTT-SN v1.2 (2013), 6.5–6.6: register an unknown normal topic before sending its publication.
        auto packet = session.publications.front();
        if (topicKind(*packet) == NORMAL_TOPIC && !session.findTopic(packet->getTopic())) {
            auto registration = MqttSnSession::makePacket(REGISTER, session.allocateMessageId()); registration->setTopic(packet->getTopic());
            transmit(registration, true); return;
        }
        session.publications.pop_front();
        auto wire = staticPtrCast<MqttSnPacket>(packet->dupShared());
        if (topicKind(*wire) == NORMAL_TOPIC) wire->setTopicId(session.findTopic(packet->getTopic()));
        wire->setTopic(""); wire->setMessageId(qos(*wire) > 0 ? session.allocateMessageId() : 0);
        transmit(wire, qos(*wire) > 0);
    }
    if ((phase != ACTIVE && phase != ASLEEP) || !session.transactions.empty() || !session.publications.empty() || simTime() < session.publishAfter) return;
    while (actionIndex < actions.size() && origin >= SIMTIME_ZERO && origin + actions[actionIndex].first <= simTime()) {
        auto action = staticPtrCast<MqttSnPacket>(actions[actionIndex].second->dupShared());
        if (phase == ASLEEP && action->getType() != PINGREQ) { connectGateway(); return; }
        actionIndex++;
        int type = action->getType();
        if (type == PUBLISH) { session.publications.push_back(action); drive(); return; }
        if (type == SUBSCRIBE || type == UNSUBSCRIBE) {
            action->setMessageId(session.allocateMessageId());
            auto filter = topicKind(*action) == NORMAL_TOPIC ? std::string(action->getTopic()) : session.resolveTopic(*action);
            if (type == SUBSCRIBE) session.subscriptions[filter] = qos(*action);
            else session.subscriptions.erase(filter);
        }
        if (type == DISCONNECT) { complete(-PINGREQ); }
        if (type == PINGREQ && phase == ASLEEP) { action->setClientIdentifier(par("clientIdentifier")); changePhase(AWAKE); }
        transmit(action, true); return;
    }
}

void MqttSnClient::scheduleTimer()
{
    cancelEvent(timer);
    if (!bound || operationalState == State::NOT_OPERATING) return;
    simtime_t next = SimTime::getMaxTime();
    if (phase == DISCONNECTED) next = reconnectAt;
    for (const auto& entry : session.transactions) next = std::min(next, entry.second.deadline);
    for (simtime_t value : {searchAt, searchResponseAt, gatewayInfoAt}) if (value >= SIMTIME_ZERO) next = std::min(next, value);
    for (const auto& candidate : candidates) next = std::min(next, candidate.second.expiry);
    if ((phase == ACTIVE || phase == ASLEEP) && session.transactions.empty()) {
        if (phase == ACTIVE && !reconciliation.empty()) next = std::min(next, simTime());
        if (!session.publications.empty()) next = std::min(next, std::max(simTime(), session.publishAfter));
        else if (actionIndex < actions.size() && origin >= SIMTIME_ZERO)
            next = std::min(next, std::max(origin + actions[actionIndex].first, session.publishAfter));
    }
    if (phase == ACTIVE && !par("connectionless").boolValue() && par("keepAlive").intValue() && !session.transactions.count(-PINGREQ))
        next = std::min(next, lastSent + SimTime(par("keepAlive")));
    if (next != SimTime::getMaxTime()) scheduleAt(std::max(simTime(), next), timer);
}

void MqttSnClient::handleMessageWhenUp(cMessage *message)
{
    if (message != timer) { socket.processMessage(message); return; }
    // MQTT-SN v1.2 (2013), 6.13; bounded poll retries follow 6.14.
    for (auto it = session.transactions.begin(); it != session.transactions.end(); ++it) {
        if (it->second.deadline > simTime()) continue;
        if (it->second.retries >= par("retryCount").intValue()) {
            if (phase == CLOSING) { complete(-DISCONNECT); socket.close(); return; }
            if (phase == AWAKE && it->first == -PINGREQ) { complete(-PINGREQ); changePhase(ASLEEP); break; }
            loseGateway(); break;
        }
        auto retry = staticPtrCast<MqttSnPacket>(it->second.packet->dupShared());
        it->second.retries++; it->second.deadline = simTime() + par("retryInterval");
        if (retry->getType() == PUBLISH || retry->getType() == SUBSCRIBE) retry->setFlags(retry->getFlags() | DUPLICATE);
        transmit(retry);
    }
    for (auto it = candidates.begin(); it != candidates.end();) {
        if (it->second.expiry <= simTime()) it = candidates.erase(it); else ++it;
    }
    if (searchAt >= SIMTIME_ZERO && searchAt <= simTime()) {
        auto packet = MqttSnSession::makePacket(SEARCHGW); packet->setRadius(1);
        sendTo(packet, L3AddressResolver().resolve(par("broadcastAddress")), par("gatewayPort"));
        searchAt = -1; searchResponseAt = simTime() + par("searchResponseTimeout");
    }
    if (searchResponseAt >= SIMTIME_ZERO && searchResponseAt <= simTime()) {
        // MQTT-SN v1.2 (2013), 6.1: increase unanswered search intervals exponentially.
        if (searchBackoff <= SIMTIME_ZERO) searchBackoff = par("searchResponseTimeout");
        searchAt = simTime() + searchBackoff + uniform(0, par("searchDelay").doubleValue());
        searchBackoff = std::min(searchBackoff * 2, SimTime(par("searchRetryMax")));
        searchResponseAt = -1; attemptedGateways.clear();
    }
    if (gatewayInfoAt >= SIMTIME_ZERO && gatewayInfoAt <= simTime()) {
        gatewayInfoAt = -1;
        if (!candidates.empty()) {
            const auto& first = *candidates.begin();
            if (first.second.address.getType() == L3Address::IPv4) {
                auto packet = MqttSnSession::makePacket(GWINFO); packet->setGatewayId(first.first); packet->setGatewayAddressArraySize(5); packet->setGatewayAddress(0, 4);
                uint32_t address = first.second.address.toIpv4().getInt();
                for (int i = 0; i < 4; i++) packet->setGatewayAddress(i + 1, (address >> (24 - 8 * i)) & 255);
                sendTo(packet, L3AddressResolver().resolve(par("broadcastAddress")), par("gatewayPort"));
            }
        }
    }
    // MQTT-SN v1.2 (2013), 6.11: only active sessions send autonomous keepalive requests.
    if (phase == ACTIVE && !par("connectionless").boolValue() && par("keepAlive").intValue() && !session.transactions.count(-PINGREQ) &&
            simTime() >= lastSent + SimTime(par("keepAlive"))) transmit(MqttSnSession::makePacket(PINGREQ), true);
    drive(); scheduleTimer();
}

void MqttSnClient::socketDataArrived(UdpSocket *, Packet *message)
{
    if (message->getDataLength() > B(65507)) { delete message; return; }
    auto source = message->getTag<L3AddressInd>()->getSrcAddress();
    int port = message->getTag<L4PortInd>()->getSrcPort();
    try {
        auto packet = MqttSnPacketSerializer::decode(message->peekDataAsBytes()->getBytes());
        emit(registerSignal("packetReceivedFromLower"), message); receiveControlPacket(packet, source, port);
    }
    catch (cRuntimeError& error) { EV_WARN << error.what() << endl; }
    delete message;
    if (phase != CLOSING) drive();
    scheduleTimer();
}

void MqttSnClient::socketErrorArrived(UdpSocket *, Indication *indication) { delete indication; }
void MqttSnClient::socketClosed(UdpSocket *) { bound = false; cancelEvent(timer); if (operationalState == State::STOPPING_OPERATION) finishActiveOperation(); }
void MqttSnClient::handleMessageWhenDown(cMessage *message) { delete message; }

void MqttSnClient::handleStopOperation(LifecycleOperation *)
{
    cancelEvent(timer);
    if (effectiveClean) session.clearExchanges();
    else for (auto it = session.transactions.begin(); it != session.transactions.end();) {
        if (it->second.packet->getType() != PUBLISH && it->second.packet->getType() != PUBREL) it = session.transactions.erase(it);
        else { it->second.deadline = SimTime::getMaxTime(); ++it; }
    }
    delayActiveOperationFinish(par("stopOperationTimeout"));
    reconcileOnConnect = true;
    if (bound && !gateway.isUnspecified() && phase != DISCONNECTED && !par("connectionless").boolValue()) {
        changePhase(CLOSING); transmit(MqttSnSession::makePacket(DISCONNECT), true); scheduleTimer();
    }
    else socket.close();
}

void MqttSnClient::handleCrashOperation(LifecycleOperation *)
{
    cancelEvent(timer); socket.destroy(); bound = false;
    session.clearExchanges(); session.topics.clear(); session.subscriptions.clear(); candidates.clear(); attemptedGateways.clear();
    origin = -1; actionIndex = 0; searchAt = searchResponseAt = gatewayInfoAt = -1; resetSession = true; changePhase(DISCONNECTED);
    reconciliation.clear(); reconcileOnConnect = false;
}

void MqttSnClient::handleActiveOperationTimeout(cMessage *) { cancelEvent(timer); complete(-DISCONNECT); socket.destroy(); bound = false; finishActiveOperation(); }

} // namespace inet::mqttsn
